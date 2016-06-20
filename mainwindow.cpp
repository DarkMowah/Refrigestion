#include "mainwindow.h"
#include "ui_mainwindow.h"
#define NB_COLONNE_MAX 4
#define TAILLE_GROUPE_WIDGETS 0.2 // En fraction d'écran
#define TAILLE_GRILLE 0.7 // En fraction d'écran
#define LONGUEUR_COLONNE 200
#define HAUTEUR_LIGNE 200

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    screenWidth = QApplication::desktop()->screenGeometry().width();
    screenHeight = QApplication::desktop()->screenGeometry().height();
    this->showFullScreen();
    ui->widgets->setFixedWidth(screenWidth * TAILLE_GROUPE_WIDGETS);
    int tab = (screenWidth * TAILLE_GROUPE_WIDGETS *4)/2 - 14;
    tri = 1;
    ui->tabWidget->setStyleSheet("QTabBar::tab { height: 20px; width: "+ QString::number(tab) + "px; }");
    ui->tabWidget->setElideMode(Qt::ElideRight);

    ui->boutonAjoutIngredient->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui->boutonAjoutIngredient->setLayoutDirection(Qt::RightToLeft);
    connect(ui->boutonAjoutIngredient, SIGNAL(clicked(bool)), this, SLOT(ouvrirFenetreAjoutIngredient()));
    ui->boutonAjoutRecette->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui->boutonAjoutRecette->setLayoutDirection(Qt::RightToLeft);
    connect(ui->boutonAjoutRecette, SIGNAL(clicked(bool)), this, SLOT(ouvrirFenetreAjoutRecette()));
    connect(ui->rb_tri_alphabet, SIGNAL(toggled(bool)), this, SLOT(triAlphabetique()));
    connect(ui->rb_tri_date, SIGNAL(toggled(bool)), this, SLOT(triDatePeremption()));
    connect(ui->rb_tri_categorie, SIGNAL(toggled(bool)), this, SLOT(triCategorie()));
    connect(ui->cb_afficher_type, SIGNAL(currentIndexChanged(int)), this, SLOT(actualiserAffichageType(int)));
    fenAR = new FenetreAjoutRecette(this);
    fenAI = new FenetreAjoutIngredient(this);
    grilleIngredients = new QGridLayout();
    grilleRecettes = new QGridLayout();
    ui->verticalLayout->addLayout(grilleIngredients);
    ui->verticalLayout_2->addLayout(grilleRecettes);
    creerVignettesIngredientDemarrage();
    creerVignettesRecettesDemarrage();
    GestionDeFichiers::creerRecette("recette.rfg");
    creerPostit();
    connect(ui->textEdit_Postit, SIGNAL(textChanged()), this, SLOT(modifierContenuPostit()));
    updateHeure();
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateHeure()));
    timer->start(1000);
    ui->lcdNumberHeure->setFixedWidth(screenWidth * TAILLE_GROUPE_WIDGETS * 0.50);
    ui->lcdNumberHeure->setFixedHeight(30);
    ui->lcdNumberHeure->setSegmentStyle(QLCDNumber::Flat);
    ui->widgets->layout()->setAlignment(ui->lcdNumberHeure,Qt::AlignHCenter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ouvrirFenetreAjoutIngredient()
{
    fenAI = new FenetreAjoutIngredient(this);
    fenAI->show();
}

void MainWindow::ouvrirFenetreAjoutRecette()
{
    fenAR = new FenetreAjoutRecette(this);
    fenAR->show();
}

void MainWindow::ouvrirFenArModif(Recette* recette)
{
    fenAR = new FenetreAjoutRecette(this);
    fenAR->setContenu(recette);
    fenAR->show();
}

void MainWindow::ajoutIngredient()
{

    Ingredient* nouvelIngredient = fenAI->creerIngredient();
    if(nouvelIngredient->getNom() == "")
    {
            QMessageBox msgBox;
            msgBox.setWindowFlags(Qt::Popup);
            msgBox.setText("Erreur. Veuillez renseigner le champ 'Nom'.");
            msgBox.exec();
    }
    else {
        fenAI->close();
        fenAI = new FenetreAjoutIngredient(this);
        VignetteIngredient *newVignetteIngredient = new VignetteIngredient(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX,nouvelIngredient,this);
        grilleIngredients->addWidget(newVignetteIngredient, ingredients.size() / NB_COLONNE_MAX, ingredients.size() % NB_COLONNE_MAX);
        vignettesIngredients << newVignetteIngredient;
        ingredients << nouvelIngredient;
        switch(tri)
        {
            case 1:
                triAlphabetique();
                break;
            case 2:
                triDatePeremption();
                break;
            case 3:
                triCategorie();
        }
    }
}
void MainWindow::modifRecette(QString nomModif)
{
        foreach(VignetteRecette* vr, vignettesRecettes)
        {
            if(vr->getRecette()->getNom() == nomModif)
            {
                supprimerVignetteRecette(vr,true);

            }
        }
        //Ajout de la recette
        QString nomEntre =  fenAR->getNomEntre();

        if(!GestionDeFichiers::nomEstConforme(nomEntre))
        {
            QMessageBox msgBox;
            msgBox.setWindowFlags(Qt::Popup);
            msgBox.setText("Erreur. Le nom de la recette est non conforme ou la recette existe déjà.");
            fenAR->show();
            msgBox.exec();
        }
        else if(fenAR->pasDingredients())
        {
            QMessageBox msgBox;
            msgBox.setWindowFlags(Qt::Popup);
            msgBox.setText("Erreur. Veuillez renseigner au moins un ingrédient.");
            msgBox.exec();
        }
        else
        {
           Recette* nouvelleRecette = fenAR->creerRecette();
           fenAR->close();
           VignetteRecette *newVignetteRecette = new VignetteRecette(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX,nouvelleRecette,this);
           grilleRecettes->addWidget(newVignetteRecette, recettes.size() / NB_COLONNE_MAX, recettes.size() % NB_COLONNE_MAX);
           vignettesRecettes << newVignetteRecette;
           recettes << nouvelleRecette;
        }
}

void MainWindow::ajoutRecette()
{
    //Ajout de la recette
    QString nomEntre =  fenAR->getNomEntre();
    if(GestionDeFichiers::recetteExisteDeja(nomEntre+".rfg") || !GestionDeFichiers::nomEstConforme(nomEntre))
    {
        QMessageBox msgBox;
        msgBox.setWindowFlags(Qt::Popup);
        msgBox.setText("Erreur. Le nom de la recette est non conforme ou la recette existe déjà.");
        fenAR->show();
        msgBox.exec();
    }
    else if(fenAR->pasDingredients())
    {
        QMessageBox msgBox;
        msgBox.setWindowFlags(Qt::Popup);
        msgBox.setText("Erreur. Veuillez renseigner au moins un ingrédient.");
        msgBox.exec();
    }
    else
    {

       Recette* nouvelleRecette = fenAR->creerRecette();
       fenAR->close();
       fenAR = new FenetreAjoutRecette(this);
       VignetteRecette *newVignetteRecette = new VignetteRecette(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX,nouvelleRecette,this);
       grilleRecettes->addWidget(newVignetteRecette, recettes.size() / NB_COLONNE_MAX, recettes.size() % NB_COLONNE_MAX);
       vignettesRecettes << newVignetteRecette;
       recettes << nouvelleRecette;
    }

}

void MainWindow::creerVignettesIngredientDemarrage()
{
    QList<Ingredient*> listIng = GestionDeFichiers::listeIngredientsFichier();

    if (listIng.size() != 0)
    {
        foreach (Ingredient* ing, listIng)
        {
            VignetteIngredient *newVignetteIngredient = new VignetteIngredient(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX, ing, this);
            grilleIngredients->addWidget(newVignetteIngredient, ingredients.size() / NB_COLONNE_MAX, ingredients.size() % NB_COLONNE_MAX);
            vignettesIngredients << newVignetteIngredient;
            ingredients << ing;
        }

    }
    triAlphabetique();

}

void MainWindow::creerVignettesRecettesDemarrage()
{
    QDir rep("Recettes");
    QStringList filters;
    filters << "*.rfg";
    QStringList listeRep = rep.entryList(filters);
    if(listeRep.size()!=0)
    {
        foreach(QString chemin, listeRep)
        {
            Recette* nouvelleRecette = GestionDeFichiers::creerRecette(chemin);
            VignetteRecette *newVignetteRecette = new VignetteRecette(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX,nouvelleRecette,this);
            grilleRecettes->addWidget(newVignetteRecette, recettes.size() / NB_COLONNE_MAX, recettes.size() % NB_COLONNE_MAX);
            vignettesRecettes << newVignetteRecette;
            recettes << nouvelleRecette;
        }
    }
}

bool MainWindow::supprimerVignetteIngredient(VignetteIngredient *vignette)
{
    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::Popup);
    msgBox.setText("Vous êtes sur le point de supprimer définitivement cet ingrédient.\nConfirmer ?");
    QPushButton *yesButton = msgBox.addButton(trUtf8("Oui"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(trUtf8("Non"), QMessageBox::NoRole);
    msgBox.setDefaultButton(noButton);
    if(msgBox.exec() == 0)
    {
        ingredients.removeOne(vignette->getIngredient());
        updateVignettes();

    }
    else
    {
        return false;
    }
    reecrireFichier();
    return true;
}

bool MainWindow::supprimerVignetteRecette(VignetteRecette *vignette, bool modif)
{
    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::Popup);
    if(modif)
    {
        msgBox.setText("Vous êtes sur le point de modifier cette recette.\nConfirmer ?");
    }
    else
    {
        msgBox.setText("Vous êtes sur le point de supprimer définitivement cette recette.\nConfirmer ?");
    }
    QPushButton *yesButton = msgBox.addButton(trUtf8("Oui"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(trUtf8("Non"), QMessageBox::NoRole);
    msgBox.setDefaultButton(noButton);
    if(msgBox.exec() == 0)
    {
        GestionDeFichiers::supprimerFichierRecette(vignette->getRecette());
        recettes.removeOne(vignette->getRecette());
        foreach(VignetteRecette *vignette, vignettesRecettes) vignette->deleteLater();
        vignettesRecettes.clear();
        delete grilleRecettes;
        grilleRecettes = new QGridLayout();
        ui->verticalLayout_2->addLayout(grilleRecettes);
        foreach(Recette *recette, recettes)
        {
            VignetteRecette *newVignetteRecette = new VignetteRecette(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX, recette, this);
            grilleRecettes->addWidget(newVignetteRecette, vignettesRecettes.size() / NB_COLONNE_MAX, vignettesRecettes.size() % NB_COLONNE_MAX);
            vignettesRecettes << newVignetteRecette;
        }
    }
    else
    {
        return false;
    }
    return true;
}

void MainWindow::reecrireFichier()
{
    GestionDeFichiers::reecrireFichier(ingredients);
}
void MainWindow::creerPostit()
{
    QFontDatabase::addApplicationFont("Images/crackedJohnnie.ttf");

    QGridLayout * grillePostit = new QGridLayout();
    QFont fontPostit("Cracked Johnnie",16);
    QFile fichierPostit("postit.rfg");
    fichierPostit.open(QIODevice::ReadOnly);
    QString contenuPostit = fichierPostit.readAll();
    ui->label_Postit->setFixedSize(screenWidth * TAILLE_GROUPE_WIDGETS-10,screenWidth * TAILLE_GROUPE_WIDGETS-10);
    ui->label_Postit->setStyleSheet("QLabel#label_Postit{ border-image: url(Images/ZcPostIt.png) 0 0 0 0 stretch stretch; }");
    ui->label_Postit->setLayout(grillePostit);

    ui->textEdit_Postit->setFixedSize(screenWidth * TAILLE_GROUPE_WIDGETS -45,screenWidth * TAILLE_GROUPE_WIDGETS -150);
    ui->textEdit_Postit->setStyleSheet("background-color: transparent;");
    ui->textEdit_Postit->setFrameShape(QFrame::NoFrame);
    ui->textEdit_Postit->setFont(fontPostit);
    ui->textEdit_Postit->setText(contenuPostit);

    grillePostit->addWidget(ui->textEdit_Postit);
    fichierPostit.close();
}

void MainWindow::modifierContenuPostit()
{
    QString contenuPostit = ui->textEdit_Postit->toPlainText();
    QFile fichierPostit("postit.rfg");
    fichierPostit.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream fluxPostit(&fichierPostit);
    fluxPostit << contenuPostit;
    fichierPostit.close();
}

QList<Ingredient*> MainWindow::getIngredients()
{
    return this->ingredients;
}

void MainWindow::actualiserVignettesRecettes()
{
    foreach(VignetteRecette* vr, vignettesRecettes)
    {
        vr->actualiserAffichage();
    }
}

void MainWindow::updateHeure()
{
    ui->lcdNumberHeure->setDigitCount(8);
    ui->lcdNumberHeure->display(QTime::currentTime().toString());
    if(QTime::currentTime().toString() == "00:00:00")
    {
        foreach(VignetteIngredient *vignetteIngredient, vignettesIngredients)
        {
            vignetteIngredient->verifierPeremption();
        }
    }
}

bool MainWindow::nameLessThan(Ingredient *ing1,Ingredient *ing2)
{
    return ing1->getNom() < ing2->getNom();
}

bool MainWindow::dateLessThan(Ingredient *ing1,Ingredient *ing2)
{
    return ing1->getDate() < ing2->getDate();
}

bool MainWindow::categoryLessThan(Ingredient *ing1,Ingredient *ing2)
{
    return ing1->getType() < ing2->getType();
}

void MainWindow::triAlphabetique()
{
    tri = 1;
    qSort(ingredients.begin(), ingredients.end(), nameLessThan);
    updateVignettes();
}

void MainWindow::triDatePeremption()
{
    tri = 2;
    qSort(ingredients.begin(), ingredients.end(), dateLessThan);
    updateVignettes();
}

void MainWindow::triCategorie()
{
    tri = 3;
    qSort(ingredients.begin(), ingredients.end(), categoryLessThan);
    updateVignettes();
}

void MainWindow::actualiserAffichageType(int type)
{
    foreach(Ingredient *ingredient, ingredients)
    {
        if(type == 0 || ingredient->getType() == (type - 1))
        {
            ingredient->setAffiche(true);
        }
        else
        {
            ingredient->setAffiche(false);
        }
    }
    if(type == 0)
    {
        ui->rb_tri_categorie->setDisabled(false);

    }
    else
    {
        ui->rb_tri_categorie->setDisabled(true);
        ui->rb_tri_alphabet->setChecked(true);
    }
    updateVignettes();
}

void MainWindow::updateVignettes()
{
    foreach(VignetteIngredient *vignette, vignettesIngredients) vignette->deleteLater();
    vignettesIngredients.clear();
    delete grilleIngredients;
    grilleIngredients = new QGridLayout();
    ui->verticalLayout->addLayout(grilleIngredients);

    foreach(Ingredient *ingredient, ingredients)
    {
        if(ingredient->getAffiche())
        {
            VignetteIngredient *newVignetteIngredient = new VignetteIngredient(screenWidth * TAILLE_GRILLE / NB_COLONNE_MAX, ingredient, this);
            grilleIngredients->addWidget(newVignetteIngredient, vignettesIngredients.size() / NB_COLONNE_MAX, vignettesIngredients.size() % NB_COLONNE_MAX);
            vignettesIngredients << newVignetteIngredient;
        }
    }
}
