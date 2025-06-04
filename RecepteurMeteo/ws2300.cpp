#include "ws2300.h"

/**
 * @brief WS2300::WS2300
 * @param _listeDonnees Liste des données fournies par le capteur
 * @param _infosCapteur Liste des informations concernant le capteur
 * @param _laBDD Pointeur permettant un accès constant à la BDD
 * @param _diffusion Pointeur permettant un accès constant au serveur WebSocket
 * @details Constructeur de la classe, ajoute les premières données aux tableaux et attribut les paramètre à la classe virtuel Capteurs
 */
WS2300::WS2300(QString &_listeDonnees, const QStringList &_infosCapteur, AccesBDD &_laBDD, DiffuseurMeteo &_diffusion):
    Capteurs(_infosCapteur, _laBDD, _diffusion),
    precipitationsDerniereDemiHeure(_listeDonnees.split(';').at(3).split(',').at(0).toFloat())
{
    qDebug() << "anciennes précipitaions : " << precipitationsDerniereDemiHeure << _listeDonnees.split(';').at(3).split(',').at(0).toFloat();
    /* Ajout des premières données aux tableaux */
    AjouterDonnees(_listeDonnees);

    /* Prépare le timer pour la réception */
    delaiReception.setSingleShot(true);
}

/**
 * @brief WS2300::~WS2300
 * @details Destructeur de la classe
 */
WS2300::~WS2300()
{

}

/**
 * @brief WS2300::AjouterDonnees
 * @param _donnees Liste des données à ajouter aux tableaux
 * @details Fonction virtuel, permet l'ajout de nouvelles données dans les tableaux
 */
void WS2300::AjouterDonnees(const QString &_donnees)
{
    /* Si une trame n'a pas déjà été reçue récemment */
    if(!delaiReception.isActive()) {

        /* Lecture des données en entrées */
        QStringList listeDonnees = _donnees.split(';'), donneesTemperature(listeDonnees.at(1).split(','));
        QStringList donneesVent(listeDonnees.at(2).split(',')), donneesPrecipitation(listeDonnees.at(3).split(','));

        /* Ajout des données dans les tableaux */
        /* Température/Hygro */
        tableauHumidite.append(donneesTemperature.at(0).toFloat());
        tableauTemperature.append(donneesTemperature.at(1).toFloat());

        /* Vents */
        tableauVentMoy.append(donneesVent.at(0).toFloat());
        tableauDirectionVent.append(donneesVent.at(1).toInt());

        if(donneesVent.at(2).toFloat() > ventMax)
            ventMax = donneesVent.at(2).toFloat();

        /* Précipitations */
        nouvellesPrecipitations = donneesPrecipitation.at(0).toFloat();

        /* Démare le timer permettant de ne pas recevoir plusieurs fois la même trame */
        delaiReception.start(30000);

        qDebug() << "Donnees Ajoutees";
    } else {
        qDebug() << "Trame trop récente";
    }
}

/**
 * @brief WS2300::EnregistrerDonnees
 * @details Fonction virtuel, permet l'enregistrement des données dans la BDD
 */
void WS2300::EnregistrerDonnees()
{
    qDebug() << "------------------------------ Enregistrement des données (ws2300) ------------------------------";

    /* Faire la moyenne des données */
    qDebug() << "[Humidité]";
    float humiditeMoy = CalculerMoyenne(tableauHumidite);
    qDebug() << "[Température]";
    float temperatureMoy = CalculerMoyenne(tableauTemperature);
    qDebug() << "[Vent]";
    float vitesseVentMoy = CalculerMoyenne(tableauVentMoy);

    qDebug() << "Humidité : " << humiditeMoy << "; Tempertaure : " << temperatureMoy << "; Vitesse : " << vitesseVentMoy;

    QMap<int, int> freqMap;

    /* Remplir la Map avec les fréquences */
    for (int value : tableauDirectionVent) {
        freqMap[value]++;
    }

    /* Trouver l'élément avec la fréquence la plus élevée */
    int mostFrequentValue = tableauDirectionVent[0];
    int maxFrequency = freqMap[mostFrequentValue];

    for (auto it = freqMap.begin(); it != freqMap.end(); ++it) {
        if (it.value() > maxFrequency) {
            mostFrequentValue = it.key();
            maxFrequency = it.value();
        }
    }

    qDebug() << "[ Valeur Max du vent ]" << mostFrequentValue;

    /* préparation de la date actuelle et de l'id courant du capteur */
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    QString id = QString::number(idCourant);

    /* Préparation des données de température et hygro */
    QStringList donneesTemperature(date);
    donneesTemperature.append(QString::number(humiditeMoy));
    donneesTemperature.append(QString::number(temperatureMoy));
    donneesTemperature.append(id);

    /* Préparation des données de liès aux vents */
    QStringList donneesVent(date);
    donneesVent.append(QString::number(vitesseVentMoy));
    donneesVent.append(QString::number(mostFrequentValue));
    donneesVent.append(QString::number(ventMax));
    donneesVent.append(id);

    /* Préparation des données de précipitation */
    QStringList donneesPrecipitation(date);
    donneesPrecipitation.append(QString::number(precipitations));
    donneesPrecipitation.append(id);

    /* Erreur à noter */
    //donneesPrecipitation.append(QString::number(std::accumulate(tableauVentMax.constBegin(), tableauVentMax.constEnd(), 0)));

    /* Ajout des données dans la base */
    laBDD.EnregistrerTemperatureHygro(donneesTemperature);
    laBDD.EnregistrerVent(donneesVent);
    laBDD.EnregistrerPrecipitation(donneesPrecipitation);

    /* Remise à zéro des tableaux et de ventMax*/
    tableauHumidite.clear();
    tableauTemperature.clear();
    tableauVentMoy.clear();
    tableauDirectionVent.clear();
    ventMax = 0;

    qDebug() << "Envoie des données";
}

/**
 * @brief CapteursThermoHygro::CalculerMoyenne
 * @param _tableau Tableau contenenant les données
 * @return Retourne un floattant étant la moyenne de la totalité des données dans le tableau
 * @details Fonction virtuel, permet le calcul de la moyenne des données d'un tableau
 */
float WS2300::CalculerMoyenne(const QVector<float>& _tableau)
{
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
void WS2300::DiffuserDonnees(const int &_cle, QString &_donnees)
{
    qDebug() << "------------------------------ Diffusion des données Station ------------------------------";

    /* Récupère les données à remplacer */
    float donneesARemplacer = _donnees.split(';').at(2).split(',').at(0).toFloat();

    /* Remplace les données récupérées précédemment et les diffusent */
    _donnees.replace(QString::number(donneesARemplacer),QString::number(precipitations));
    diffusion.DiffuserTrame(_cle, _donnees);
}

/**
 * @brief WS2300::CalculerPrecipitationsDemiHeure
 * @return Retourne la valeur des précipitations sur une demi-heure à partir des attributs de la classe
 * @details Calcul la précipitation en fonction de l'ancienne valeur totale
 */
void WS2300::CalculerPrecipitationsDemiHeure()
{
    qDebug() << "------------------------------ Calcul des précipitations (Demi-heure) ------------------------------";

    /* Calcul des précipitations */
    if(precipitationsDerniereDemiHeure > 900 && nouvellesPrecipitations < 100) {
        precipitations = 999.99 - precipitationsDerniereDemiHeure + nouvellesPrecipitations;
    } else {
        precipitations = nouvellesPrecipitations - precipitationsDerniereDemiHeure;
    }

    /* Met à jour la valeur precipitationsDerniereDemiHeure */
    precipitationsDerniereDemiHeure = nouvellesPrecipitations;

    qDebug() << precipitationsDerniereDemiHeure;

}
