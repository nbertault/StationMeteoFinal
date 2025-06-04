#ifndef WS2300_H
#define WS2300_H

#include "capteurs.h"

class WS2300 : public Capteurs
{
public:
    WS2300(QString &_listeDonnees, const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion);
    ~WS2300();
    void AjouterDonnees(const QString &_donnees);
    void EnregistrerDonnees();
    float CalculerMoyenne(const QVector<float>& _tableau);
    void DiffuserDonnees(const int &_cle, QString &_donnees);
private:
    void CalculerPrecipitationsDemiHeure();

    /**
     * @details Flottant définnissant la vitesse maximal atteinte par le vent dans un certain interval
     */
    float ventMax;

    /**
     * @details Dernière précipitation reçue
     */
    float nouvellesPrecipitations;

    /**
     * @details Première précipitation reçue lors de la dernière demie-heure
     */
    float precipitationsDerniereDemiHeure;

    /**
     * @details Précipitation reçue précédemment
     */
    float precipitations;

    /**
     * @details Tableau regroupant les valeurs de température
     */
    QVector<float> tableauTemperature;

    /**
     * @details Tableau regroupant les valeurs moyennes de la vitesse du vent
     */
    QVector<float> tableauVentMoy;

    /**
     * @details Tableau regroupant les valeurs d'humidité
     */
    QVector<float> tableauHumidite;

    /**
     * @details Tableau regroupant les valeurs de direction du vent
     */
    QVector<float> tableauDirectionVent;
};

#endif // WS2300_H
