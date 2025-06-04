#include "diffuseurmeteo.h"

/**
 * @brief DiffuseurMeteo::DiffuseurMeteo
 * @details Constructeur de la classe, permet l'ouverture du serveur WebSocket
 */
DiffuseurMeteo::DiffuseurMeteo()
{
    server = new QWebSocketServer("Serveur WebSocket", QWebSocketServer::NonSecureMode);

    /* Ouvre la connection au serveur WebSocket */
    if (!server->listen(QHostAddress::Any, 12346)) {
        qDebug() << "Échec du démarrage du serveur WebSocket.";
    } else {
        qDebug() << "Serveur WebSocket démarré sur le port" << server->serverPort();
        connect(server, &QWebSocketServer::newConnection, this, &DiffuseurMeteo::onNewConnection);
    }
}

/**
 * @brief DiffuseurMeteo::DiffuserTrame
 * @param _id Id définie avec les clients pour identifier les capteurs
 * @param _donnees Données à transmettre aux clients
 * @details Diffuse une trame construite à partir des données et de la clé fournies en attributs à tous les clients
 */
void DiffuseurMeteo::DiffuserTrame(const int &_id, const QString &_donnees)
{
    qDebug() << "Il y a " << listeClients.size() << " client(s)";

    /* Parcours la liste des clients */
    if (listeClients.size() != 0) {
        for (QWebSocket *client : listeClients)
        {
            if (client && client->isValid())
            {
                client->sendTextMessage(CreerTrame(_id, _donnees));
            }
        }
    }
}

/**
 * @brief DiffuseurMeteo::CreerTrame
 * @param _id Id définie avec les clients pour identifier les capteurs
 * @param _donnees Données à inscrire dans une trame
 * @return Retourne une trame JSON sous forme d'une QString
 * @details Créer une trame JSON qui sera communiquer aux clients WebSocket
 */
QString DiffuseurMeteo::CreerTrame(const int &_id, const QString &_donnees)
{
    QJsonObject trame;

    qDebug() << "------------------------------ Création d'une trame JSON ------------------------------";
    /* Sépare les données dans une liste de données */
    QStringList listeDonnees = _donnees.split(';');

    /* Lis la date actuelle */
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QStringList donneesTemperature(listeDonnees.at(1).split(','));

    /* Intègre la date, l'id, l'état de la batterie, l'humidité et la température à la trame (Données communes à tous les capteurs) */
    trame["Date"]=date;
    trame["Identifiant"]=_id;
    trame["Etat_Batterie"]= listeDonnees.at(0);
    trame["Humidite"]=donneesTemperature.at(0).toFloat();
    trame["Temperature"]=donneesTemperature.at(1).toDouble();

    /* Si c'est la station météo */
    if(_id == 4){
        QStringList donneesVent(listeDonnees.at(2).split(','));
        QStringList donneesPrecipitation(listeDonnees.at(3).split(','));

        /* Intègre les données liées au vents et aux précipitations */
        trame["Vent_Moyen"]=donneesVent.at(0).toFloat();
        trame["Direction_Vent"]=donneesVent.at(1).toInt();
        trame["Rafale"]=donneesVent.at(2).toFloat();
        trame["Precipitations"]=donneesPrecipitation.at(0).toFloat();

        // tableauPrecipitations.append(CalculerPrecipitations(donneesPrecipitation.at(0).toFloat()));
    }

    QJsonDocument doc(trame);
    qDebug() << doc;
    return QString(doc.toJson(QJsonDocument::Compact));
}

/**
 * @brief DiffuseurMeteo::onNewConnection
 * @details Ajoute un client à la liste de clients lorsqu'un nouveau client se connecte
 */
void DiffuseurMeteo::onNewConnection()
{
    QWebSocket *client = server->nextPendingConnection();
    qDebug() << "Nouveau client connecté.";

    /* Ajout des clients à la liste */
    listeClients.append(client);

    /* Suppression du client si il se déconnecte */
    connect(client, &QWebSocket::disconnected, this, [this, client]() {
        qDebug() << "Client déconnecté.";
        listeClients.removeAll(client);
        client->deleteLater();
    });
}
