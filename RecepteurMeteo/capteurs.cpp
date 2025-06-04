#include "capteurs.h"

/**
 * @brief Capteurs::Capteurs
 * @param _infosCapteur Liste des informations du concernant le capteur
 * @details Constructeur de la classe, attribut les valeurs concernant le capteur, appelle la classe AccesBDD et
 * demande la création d'un capteur dans la BDD
 */
Capteurs::Capteurs(const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion):
    modele(_infosCapteur.at(0)),
    idCourant(_infosCapteur.at(1).toInt()),
    canal(_infosCapteur.at(2).toInt()),
    donneesAjoutees(false),
    donneesADiffuser(false),
    laBDD(_laBDD),
    diffusion(_diffusion)
{
    /* Ajout du capteur à la BDD */
    qDebug() << modele;
    if (laBDD.AjouterCapteur(idCourant, canal, modele, id) == true)
        qDebug() << "Capteur ajouté ou déja present avec l'id :" << id;
    else
        qDebug() << "Erreur lors de l'ajout";
}

/**
 * @brief Capteurs::~Capteurs
 * @details Destructeur virtuel de la classe
 */
Capteurs::~Capteurs()
{

}
