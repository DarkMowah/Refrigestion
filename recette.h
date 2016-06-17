#ifndef RECETTE_H
#define RECETTE_H

#include <QObject>
#include <ingredient.h>

class Recette
{
public:
    explicit Recette(QString nom, QString dureePrep, QList<QString> listeIng,QString etapesPrep,QString typeRecette, QString nomImage, QString cheminImage);
    QString getNom();
    QString getDureePreparation();
    QString getCheminImage();
    QString getNomImage();
    QString getEtapesPreparation();
    void creerFichierRecette();
    QList<QString> getListIngredients();
    QString getTypeRecette();
    bool aucunIngredient();
    ~Recette();
private :
    QString nom;
    QString dureePreparation;
    QList<QString> listeIngredients;
    QString etapesPreparation;
    QString typeRecette;
    QString cheminImage;
    QString nomImage;
};

#endif // RECETTE_H
