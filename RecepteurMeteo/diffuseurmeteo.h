#ifndef DIFFUSEURMETEO_H
#define DIFFUSEURMETEO_H

#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonDocument>
#include <QJsonObject>

class DiffuseurMeteo : public QObject
{
public:
    DiffuseurMeteo();
    void DiffuserTrame(const int &_id, const QString &_donnees);
    QString CreerTrame(const int &_id, const QString &_donnees);
private slots:
    void onNewConnection();
private :
    /**
     * @details Pointeur du serveur WebSocket
     */
    QWebSocketServer *server;

    /**
     * @details List de pointeur de clients WebSocket
     */
    QList<QWebSocket *> listeClients;
};

#endif // DIFFUSEURMETEO_H
