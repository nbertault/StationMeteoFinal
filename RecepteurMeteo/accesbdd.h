#ifndef ACCESBDD_H
#define ACCESBDD_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QDir>
#include <QDebug>

class AccesBDD
{
public:
    AccesBDD();
    bool AjouterCapteur(const int _idCourant, const int _canal, const QString _modele, int &_id);
    void EnregistrerTemperatureHygro(const QStringList &_donnees);
    void EnregistrerVent(const QStringList &_donnees);
    void EnregistrerPrecipitation(const QStringList &_donnees);
    int GetId(const int _idCourant);
private:
    /**
     * @details Instance de la BDD
     */
    QSqlDatabase db;

    /**
     * @details Fichier contenant les informations de la BDD
     */
    QSettings *fichierINI;

    /**
     * @details Requête qui servira dans la gestion de la BDD
     */
    QSqlQuery *requete;


};

#endif // ACCESBDD_H
