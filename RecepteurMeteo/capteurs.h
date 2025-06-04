#ifndef CAPTEURS_H
#define CAPTEURS_H

#include "accesbdd.h"
#include "diffuseurmeteo.h"
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <QVector>

class Capteurs : public QObject
{

public:
    Capteurs(const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion);
    virtual ~Capteurs();
    virtual void AjouterDonnees(const QString &_donnees) = 0;
    virtual void EnregistrerDonnees() = 0;
    virtual float CalculerMoyenne(const QVector<float>& _tableau) = 0;
    virtual void DiffuserDonnees(const int &_cle, QString &_donnees) = 0;

    /**
     * @details Id fourni par le capteur
     */
    int idCourant;

    /**
     * @details Id du capteur sur la BDD
     */
    int id;

    /**
     * @details Booléan permettant de savoir si de nouvelles données ont été ajoutées depuis
     * le dernier enregistremment
     */
    bool donneesAjoutees;

    /**
     * @details Booléan permettant de savoir si il faut diffuser des données
     */
    bool donneesADiffuser;
protected:
    /**
     * @details Canal sur lequel le capteur opère
     */
    int canal;

    /**
     * @details Liste des données liées au capteur
     */
    QString donnees;

    /**
     * @details Nom du capteur
     */
    QString modele;

    /**
     * @details Référence de la BDD
    */
    AccesBDD &laBDD;

    /**
     * @details Référence du serveur WebSocket
    */
    DiffuseurMeteo &diffusion;

    /**
     * @details Timer permettant de ne pas traiter plusieurs fois la même trame
    */
    QTimer delaiReception;
};

#endif // CAPTEURS_H
