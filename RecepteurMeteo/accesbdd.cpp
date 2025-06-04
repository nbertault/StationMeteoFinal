#include "accesbdd.h"

/**
 * @brief AccesBDD::AccesBDD
 * @details Constructeur de la classe, ouvre le fichier contenant les informations de la BDD et
 * ouvre la connexion à la BDD
 */
AccesBDD::AccesBDD()
{
    /* Ouverture du fichier contenant les infromations de la base de données */
    QString filePath = "/home/pi/Documents/build-RecepteurMeteo-Desktop-Debug/config.ini";
    fichierINI = new QSettings(filePath, QSettings::IniFormat);

    /* Insertion des paramètres et ouverture de la connexion à la base de données */
    qDebug() << fichierINI->value("Informations_BDD/nomHote").toString() << fichierINI->value("Informations_BDD/nomDataBase").toString() << fichierINI->value("Informations_BDD/UserName").toString() << fichierINI->value("Informations_BDD/MotDePasse").toString();
    db = QSqlDatabase::addDatabase("QMARIADB");
    db.setHostName(fichierINI->value("Informations_BDD/nomHote").toString()); // Adresse de la base de données
    db.setDatabaseName(fichierINI->value("Informations_BDD/nomDataBase").toString()); // Nom de la base de données
    db.setUserName(fichierINI->value("Informations_BDD/UserName").toString()); // Nom d'utilisateur
    db.setPassword(fichierINI->value("Informations_BDD/MotDePasse").toString()); // Mot de passe

    if(!db.isOpen()) {
        if(db.open()) {
            qDebug() << "Connexion réussie à la base de données.";
        } else {
            qDebug() << "Erreur lors de la connexion :" << db.lastError().text();
        }
    }

    requete = new QSqlQuery(db);
}

/**
 * @brief AccesBDD::AjouterCapteur
 * @param _idCourant ID fourni par le capteur permettant son identification
 * @param _canal Canal sur lequel opère le capteur
 * @param _modele Modèle du capteur permettant son identification
 * @param _id Id liée au capteur, il est créé par la BDD et est définitif
 * @return Retourne un booléen permettant de confirmer l'ajout du capteur dans la BDD
 * @details Permet l'Ajout d'un capteur dans la BDD ou la modification de son idCourant si celui à changer
 */
bool AccesBDD::AjouterCapteur(const int _idCourant, const int _canal, const QString _modele, int &_id)
{

    bool retour;
    qDebug() << "ID Courant :" << _idCourant;
    qDebug() << "Modèle:" <<_modele;
    qDebug() << "Canal :" << _canal;

    /* Compte le nombre d'apparition du capteur dans la BDD */
    requete->clear();
    requete->prepare("SELECT COUNT(*) FROM Sensors WHERE model = :modele AND channel = :canal");
    requete->bindValue(":modele", _modele);
    requete->bindValue(":canal", _canal);

    if(requete->exec()) {
        if (requete->next()) {

            /* Vérifier si le capteur est déja créé */
            if (requete->value(0).toInt() == 0) {
                qDebug() << "------------------------------ Ajout du capteur ------------------------------";

                /* Ajout du capteur dans la base de données */
                requete->prepare("INSERT INTO Sensors (`idcurrent`, `channel`, `model`) VALUES (:idCourant, :canal, :modele)");

                /* Erreur à noter */
                //requete.prepare("INSERT INTO Sensors (`idcurrent`, `channel`, `model`) VALUES (:idcurrent, :batteryState, :channel, :model)");
            } else {
                qDebug() << "------------------------------ Changement ID Courant ------------------------------";

                /* Changement de l'id courant */
                requete->prepare("UPDATE Sensors SET `idcurrent` = :idCourant WHERE model = :modele AND channel = :canal");
            }

            requete->bindValue(":idCourant", _idCourant);
            requete->bindValue(":modele", _modele);
            requete->bindValue(":canal", _canal);

            if (requete->exec()) {
                qDebug() << "Changement ou Ajout du capteur effectué";
                retour = true;
            } else {
                qDebug() << "Erreur changement de l'id courant du capteur";
                qDebug() << requete->lastError().text();
            }

            /* Récupère l'id du capteur et le lui transmet*/
            _id = GetId(_idCourant);

        } else {
            qDebug() << ("Erreur lors de l'ajout d'un capteur : " + requete->lastError().text());
            retour = false;
        }
    }
    return retour;
}

/**
 * @brief AccesBDD::EnregistrerTemperatureHygro
 * @param _donnees Liste des données (température et hygrométrie) à inscrire dans la table temperature de la BDD
 * @details Enregistre les valeurs fournies dans la table temperature
 */
void AccesBDD::EnregistrerTemperatureHygro(const QStringList &_donnees)
{
    qDebug() << "------------------------------ Enregistrement des données de Température/Hygro ------------------------------";

    /* Préparer les données avant le stockage */
    float humidite = _donnees.at(1).toFloat(), ancienneHumidite, idtemperature;
    float temperature = _donnees.at(2).toFloat(), ancienneTemperature;
    int idCourant = _donnees.at(3).toInt();
    qDebug() << "ID :" << idCourant;
    int id = GetId(idCourant);


    /* Recherche des anciennes données et de leur id */
    requete->clear();
    requete->prepare("SELECT idtemperature, temperature, humidity FROM Temperatures WHERE idsensor = :id ORDER BY idtemperature DESC LIMIT 1");
    requete->bindValue(":id", id);
    qDebug() << "Recherche Anciennes Données pour l'id :" << id;

    if(requete->exec()) {
        if (requete->next()) {
            idtemperature = requete->value("idtemperature").toFloat();
            ancienneTemperature = requete->value("temperature").toFloat();
            ancienneHumidite = requete->value("humidity").toInt();

            /* Comparaison entre les anciennes et les nouvelles données */
            if(ancienneTemperature == temperature && ancienneHumidite == humidite) {

                /* Editer la date */
                requete->clear();
                requete->prepare("UPDATE Temperatures SET date = :date WHERE idtemperature = :idtemperature");
                requete->bindValue(":date", _donnees.at(0));
                requete->bindValue(":idtemperature", idtemperature);

                if(requete->exec())
                    qDebug() << "Date Température/Hygro mise à jour";
                else
                    qDebug() << requete->lastError().text();

            } else {
                /* Ajouter les données dans la base */
                requete->clear();
                requete->prepare("INSERT INTO Temperatures (`date`,`humidity`,`temperature`,`idsensor`) VALUES (:date, :humidity, :temperature, :idsensor)");
                requete->bindValue(":date", _donnees.at(0));
                requete->bindValue(":humidity", humidite);
                requete->bindValue(":temperature", temperature);
                requete->bindValue(":idsensor", id);

                if(requete->exec())
                    qDebug() << "Temperatures/Hygro ajoutées";
                else
                    qDebug() << requete->lastError().text();
            }
        } else {
            /* Ajouter les données dans la base */
            requete->clear();
            requete->prepare("INSERT INTO Temperatures (`date`,`humidity`,`temperature`,`idsensor`) VALUES (:date, :humidity, :temperature, :idsensor)");
            requete->bindValue(":date", _donnees.at(0));
            requete->bindValue(":humidity", humidite);
            requete->bindValue(":temperature", temperature);
            requete->bindValue(":idsensor", id);

            if(requete->exec())
                qDebug() << "Temperatures/Hygro ajoutées";
            else
                qDebug() << requete->lastError().text();
        }
    } else
        qDebug() << requete->lastError().text();
}

/**
 * @brief AccesBDD::EnregistrerVent
 * @param _donnees Liste des données (direction, vitesse moyenne et vitesse maximal du vent) à enregistrer dans la table wind de la BDD
 * @details Enregistre les valeurs fournies dans la table wind
 */
void AccesBDD::EnregistrerVent(const QStringList &_donnees)
{
    qDebug() << "------------------------------ Enregistrement des données liès aux vents ------------------------------";

    /* Préparer les données avant le stockage */
    int idCourant = _donnees.at(4).toInt(), direction = _donnees.at(2).toInt();
    float ventMoy = _donnees.at(1).toFloat(), ventMax = _donnees.at(3).toFloat();
    int id = GetId(idCourant);

    /* Ajouter les données dans la base */
    requete->clear();
    requete->prepare("INSERT INTO Winds (`date`,`windavg`,`direction`,`windmax`,`idsensor`) VALUES (:date, :ventMoy, :direction, :ventMax, :idsensor)");
    requete->bindValue(":date", _donnees.at(0));
    requete->bindValue(":ventMoy", ventMoy);
    requete->bindValue(":direction", direction);
    requete->bindValue(":ventMax", ventMax);
    requete->bindValue(":idsensor", id);

    if(requete->exec())
        qDebug() << "Vent ajoutées";
    else
        qDebug() << requete->lastError().text();
}

/**
 * @brief AccesBDD::EnregistrerPrecipitation
 * @param _donnees Liste des données (precipitations) à enregistrer dans la table precipitation de la BDD
 * @details Enregistre les valeurs fournies dans la table precipitation
 */
void AccesBDD::EnregistrerPrecipitation(const QStringList &_donnees)
{
    qDebug() << "------------------------------ Enregistrement des données de Précipitations ------------------------------";

    /* Préparer les données avant le stockage */
    int idprecipitation, idCourant = _donnees.at(2).toInt();
    float precipitations = _donnees.at(1).toFloat(), anciennesPrecipitations;
    int id = GetId(idCourant);

    /* Recherche des anciennes données et de leur id */
    requete->clear();
    requete->prepare("SELECT idprecipitation, value FROM Precipitations WHERE idsensor = :id ORDER BY idprecipitation DESC LIMIT 1");
    requete->bindValue(":id", id);

    if(requete->exec()) {
        if (requete->next()) {
            idprecipitation = requete->value("idprecipitation").toFloat();
            anciennesPrecipitations = requete->value("value").toFloat();

            /* Comparaison entre les anciennes et les nouvelles données */
            if(anciennesPrecipitations == precipitations) {
                /* Editer la date */
                requete->prepare("UPDATE Precipitations SET date = :date WHERE idprecipitation = :idprecipitation");
                requete->bindValue(":date", _donnees.at(0));
                requete->bindValue(":idprecipitation", idprecipitation);

                if(requete->exec())
                    qDebug() << "Date Precipitations mise à jour";
                else
                    qDebug() << requete->lastError().text();

            } else {
                /* Ajouter les données dans la base */
                requete->prepare("INSERT INTO Precipitations (`date`,`value`,`idsensor`) VALUES (:date, :precipitation, :idsensor)");
                requete->bindValue(":date", _donnees.at(0));
                requete->bindValue(":precipitation", precipitations);
                requete->bindValue(":idsensor", id);

                if(requete->exec())
                    qDebug() << "Precipitations ajoutées";
                else
                    qDebug() << requete->lastError().text();
            }
        } else {
            /* Ajouter les données dans la base */
            requete->prepare("INSERT INTO Precipitations (`date`,`value`,`idsensor`) VALUES (:date, :precipitation, :idsensor)");
            requete->bindValue(":date", _donnees.at(0));
            requete->bindValue(":precipitation", precipitations);
            requete->bindValue(":idsensor", id);

            if(requete->exec())
                qDebug() << "Precipitations ajoutées";
            else
                qDebug() << requete->lastError().text();
        }
    } else
        qDebug() << requete->lastError().text();

}


/**
 * @brief AccesBDD::GetId
 * @param _idCourant ID fourni par le capteur permettant son identification
 * @return Retourne un entier correspondant à la valeur de l'id fourni par la BDD
 * @details Trouve et retourne l'id présent dans la BDD à l'aide de l'id courant
 */
int AccesBDD::GetId(const int _idCourant)
{
    int retour;

    /* Récupère l'id du capteur à partir de l'id courant */
    requete->clear();
    requete->prepare("SELECT idsensor FROM Sensors WHERE `idcurrent` = :idCourant");
    requete->bindValue(":idCourant", _idCourant);

    if(requete->exec()) {
        if(requete->next()) {
            qDebug() << "id : " << requete->value(0).toInt();
            retour = requete->value(0).toInt();
        } else {
            qDebug() << ("Erreur lors de la lecture de l'id : " + requete->lastError().text());
        }
    } else {
        qDebug() << ("Erreur lors de la lecture de l'id : " + requete->lastError().text());
    }
    return retour;
}
