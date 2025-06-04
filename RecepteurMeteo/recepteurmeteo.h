#ifndef RECEPTEURMETEO_H
#define RECEPTEURMETEO_H
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>
#include "capteursthermohygro.h"
#include "ws2300.h"
#include "accesbdd.h"
#include "diffuseurmeteo.h"

class RecepteurMeteo : public QObject
{
    /**
     * @details Enumération des différents type de capteurs
     */
    enum TypeCapteurs {
        FINEOFFSET,
        NEXUS,
        OREGON,
        AUTRE
    };

public:
    RecepteurMeteo();
    ~RecepteurMeteo();
    void TraiterTrame(const QJsonDocument _trameRecue);
    TypeCapteurs TrouverCapteur(const QString _capteur);
    void CalculerDelaiTimer();
private slots:
    void readOutput();
    void onStarted();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTimer_timeout();
private:
    /**
     * @details Pointeur du processus à utiliser (RTL_433)
     */
    QProcess *process;

    /**
     * @details Map composé d'une clé en entier et d'une valeur en pointeur de Capteurs
     */
    QMap<int,Capteurs*> capteurs;

    /**
     * @details Pointeur du timer
     */
    QTimer *timer;

    /**
     * @details Pointeur du serveur WebSocket
     */
    DiffuseurMeteo *diffusion;

    /**
     * @details Pointeur de la BDD
     */
    AccesBDD *bd;
};

#endif // RECEPTEURMETEO_H
