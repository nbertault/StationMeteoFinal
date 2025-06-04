#include "capteursthermohygro.h"

/**
 * @brief CapteursThermoHygro::CapteursThermoHygro
 * @param _listeDonnees Liste des données fournies par le capteur
 * @param _infosCapteur Liste des informations concernant le capteur
 * @param _laBDD Pointeur permettant un accès constant à la BDD
 * @param _diffusion Pointeur permettant un accès constant au serveur WebSocket
 * @details Constructeur de la classe, ajoute les premières données aux tableaux et attribut les paramètre à la classe virtuel Capteurs
 */
CapteursThermoHygro::CapteursThermoHygro(QString &_listeDonnees, const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion):
    Capteurs(_infosCapteur, _laBDD, _diffusion)
{
    /* Ajout des premières données aux tableaux */
    AjouterDonnees(_listeDonnees);

    /* Prépare le timer pour la réception */
    delaiReception.setSingleShot(true);
}

/**
 * @brief CapteursThermoHygro::~CapteursThermoHygro
 * @details Destructeur de la classe
 */
CapteursThermoHygro::~CapteursThermoHygro()
{

}

/**
 * @brief CapteursThermoHygro::AjouterDonnees
 * @param _donnees Liste des données à ajouter aux tableaux
 * @details Fonction virtuel, permet l'ajout de nouvelles données dans les tableaux
 */
void CapteursThermoHygro::AjouterDonnees(const QString &_donnees)
{
    /* Si une trame n'a pas déjà été reçue récemment */
    if(!delaiReception.isActive()) {

        /* Lecture des données en entrées */
        qDebug() << "données : " << _donnees;
        QStringList listeDonnees = _donnees.split(';');

        QStringList donneesTemperature(listeDonnees.at(1).split(','));

        /* Ajout des données dans les tableaux */
        tableauHumidite.append(donneesTemperature.at(0).toFloat());
        tableauTemperature.append(donneesTemperature.at(1).toFloat());

        /* Démare le timer permettant de ne pas recevoir plusieurs fois la même trame */
        delaiReception.start(30000);

        qDebug() << "Donnees Ajoutees";

    } else {
        qDebug() << "Trame trop récente";
    }
}

/**
 * @brief CapteursThermoHygro::EnregistrerDonnees
 * @details Fonction virtuel, permet l'enregistrement des données dans la BDD
 */
void CapteursThermoHygro::EnregistrerDonnees()
{
    qDebug() << "------------------------------ Enregistrement des données (température/humidité) ------------------------------";

    /* Faire la moyenne des données */
    float humiditeMoy = CalculerMoyenne(tableauHumidite);
    float temperatureMoy = CalculerMoyenne(tableauTemperature);

    qDebug() << "Humidité : " << humiditeMoy << "; Tempertaure : " << temperatureMoy;

    /* préparation de la date actuelle et de l'id courant du capteur */
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    QString id = QString::number(idCourant);

    /* Préparation des données de température et hygro */
    QStringList donneesTemperature(date);
    donneesTemperature.append(QString::number(humiditeMoy));
    donneesTemperature.append(QString::number(temperatureMoy));
    donneesTemperature.append(id);

    /* Ajout des données dans la base */
    laBDD.EnregistrerTemperatureHygro(donneesTemperature);

    /* Remise à zéro des tableaux */
    tableauHumidite.clear();
    tableauTemperature.clear();

    qDebug() << "Envoie des données";
}

/**
 * @brief CapteursThermoHygro::CalculerMoyenne
 * @param _tableau Tableau contenenant les données
 * @return Retourne un flottant étant la moyenne de la totalité des données dans le tableau
 * @details Fonction virtuel, permet le calcul de la moyenne des données d'un tableau
 */
float CapteursThermoHygro::CalculerMoyenne(const QVector<float> &_tableau)
{
    qDebug() << "Calul de la Moyenne";

    float somme = 0;
    for(float valeur : _tableau)
        somme += valeur;

    return somme/_tableau.size();
}

/**
 * @brief CapteursThermoHygro::DiffuserDonnees
 * @param _cle Clé attribué par la classe recepteur météo permettant l'identification des capteurs par les clients WebSocket
 * @param _donnees Liste des données à diffuser aux clients WebSocket
 * @details Fonction virtuel, permet la diffusion des données grâce au serveur WebSocket
 */
void CapteursThermoHygro::DiffuserDonnees(const int &_cle, QString &_donnees)
{
    qDebug() << "------------------------------ Diffusion des données Thermo/Hygro ------------------------------";
    diffusion.DiffuserTrame(_cle, _donnees);
}
