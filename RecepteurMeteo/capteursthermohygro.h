#ifndef CAPTEURSTHERMOHYGRO_H
#define CAPTEURSTHERMOHYGRO_H

#include "capteurs.h"

class CapteursThermoHygro : public Capteurs
{
public:
    CapteursThermoHygro(QString &_listeDonnees, const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion);
    ~CapteursThermoHygro();
    void AjouterDonnees(const QString &_donnees);
    void EnregistrerDonnees();
    float CalculerMoyenne(const QVector<float>& _tableau);
    void DiffuserDonnees(const int &_cle, QString &_donnees);
private:
    /**
     * @details Tableau regroupant les valeurs de température
    */
    QVector<float> tableauTemperature;

    /**
     * @details Tableau regroupant les valeurs d'humidité
    */
    QVector<float> tableauHumidite;

};

#endif // CAPTEURSTHERMOHYGRO_H
