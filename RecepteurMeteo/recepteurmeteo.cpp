#include "recepteurmeteo.h"

/**
 * @brief RecepteurMeteo::RecepteurMeteo
 * @details Constructeur de la classe permet l'initalisation du timer, le lancement du programme
 * RTL_433, la connection des signaux, la connection aux autres classes et l'initialisation des capteurs
 */
RecepteurMeteo::RecepteurMeteo():
    bd(new AccesBDD),
    diffusion(new DiffuseurMeteo)
{
    /* Configure et lance le timer */
    timer = new QTimer(this);
    timer->setSingleShot(true);
    CalculerDelaiTimer();

    /* Configure le processus (RTL_433) */
    process = new QProcess(this);
    QString lienProgramme = "/usr/bin/rtl_433";
    QStringList arguments;
    arguments << "-F" << "json";

    /* Lance le processus (RTL_433) */
    process->start(lienProgramme, arguments);

    if (!process->waitForStarted()) {
        qDebug() << "Échec du démarrage du processus rtl_433";
    } else {
        qDebug() << "Démarrage du processus rtl_433";
    }
    qDebug() << "Commande  : " << process->program();

    /* Connecte les signaux du processus (RTL_433) et du timer */
    connect(process, &QProcess::readyReadStandardOutput, this, &RecepteurMeteo::readOutput);
//    connect(process, &QProcess::finished, this, &RecepteurMeteo::onFinished);
    connect(process, &QProcess::started, this, &RecepteurMeteo::onStarted);
    connect(timer, &QTimer::timeout, this, &RecepteurMeteo::onTimer_timeout);
}

/**
 * @brief RecepteurMeteo::~RecepteurMeteo
 * @details Destructeur de la classe
 */
RecepteurMeteo::~RecepteurMeteo()
{
    /* Arrête le processus (RTL_433) */
    process->close();
}

/**
 * @brief RecepteurMeteo::TraiterTrame
 * @param _trameRecue Fichier JSON contenant toutes les informations reçues du logiciel
 * @details Permet le traitement des données reçues des capteurs
 */
void RecepteurMeteo::TraiterTrame(const QJsonDocument _trameRecue)
{
    qDebug() << "------------------------------ Traitement des données ------------------------------";

    /* Initalise les premiers attributs */
    QString donnees;
    QStringList infosCapteur;
    int canal, cle;

    /* Si ce n'est pas un de nos capteurs */
    if (TrouverCapteur(_trameRecue.object()["model"].toString()) == AUTRE) {
        qDebug() << "Capteur non reconnu";
        cle = -1;
    } else {

        /* Récupère le modèle, l'id et les ajoutent aux informations du capteur */
        infosCapteur << _trameRecue.object()["model"].toString();
        int id = _trameRecue.object()["id"].toInt();
        infosCapteur << QString::number(id);

        qDebug() << "id : " << id;
        qDebug() << "modèle : " << _trameRecue.object()["model"].toString();

        /* Peut être fait en une ligne regarder code stage */
        /* Vérifie l'état de la batterie et l'ajoute aux données */
        if(_trameRecue.object()["battery_ok"].toInt() == 1)
            donnees += "true";
        else
            donnees += "false";

        donnees += ';';


        /* Attribut les valeurs de l'humidité et de la température */
        float humidity = _trameRecue.object()["humidity"].toDouble(), temperature = _trameRecue.object()["temperature_C"].toDouble();
        donnees += QString::number(humidity) + ',' + QString::number(temperature);

        /* Si l'appareil est un Nexus ou un Oregon */
        if(TrouverCapteur(_trameRecue.object()["model"].toString()) == NEXUS || TrouverCapteur(_trameRecue.object()["model"].toString()) == OREGON) {

            /* Définition des clés du QMap et des canaux*/
            /* Récupère le canal et définie l'attribut pour la clé*/
            canal = _trameRecue.object()["channel"].toInt();
            qDebug() << "canal : " << canal;
            cle = _trameRecue.object()["channel"].toInt();

            /* Si c'est un Oregon on ajoute +4 au canal pour avoir sa clé */
            if(TrouverCapteur(_trameRecue.object()["model"].toString()) == OREGON) {
                qDebug() << "------------------------------ Oregon Canal" << canal << "------------------------------";

                cle += 4;
            } else {
                qDebug() << "------------------------------ Nexus Canal" << canal << "------------------------------";
            }

            /* Ajoute le canal aux informations du capteur */
            infosCapteur << QString::number(canal);
        }

        /* Si l'appareil est un FineOffset */
        if (TrouverCapteur(_trameRecue.object()["model"].toString()) == FINEOFFSET) {
            qDebug() << "------------------------------ FineOffset ------------------------------";

            /* Défini le canal à 0 pour les informations du capteur */
            infosCapteur << "0";

            /* Défini la clé à 4 */
            cle = 4;

            /* Attribut les valeurs liées aux vents et aux précipitations */
            int direction = _trameRecue.object()["wind_dir_deg"].toInt();
            QString ventMoy = _trameRecue.object()["wind_avg_km_h"].toString(), ventMax = _trameRecue.object()["wind_max_km_h"].toString();
            QString precipitation = _trameRecue.object()["rain_mm"].toString();
            qDebug() << _trameRecue.object()["rain_mm"].toDouble();
            donnees += ';' + ventMoy + ',' + QString::number(direction) + ',' + ventMax + ';' + precipitation;
        }

        /* Si le capteur n'est pas encore créé ou que son id courant n'est pas le bon*/
        if(capteurs.value(cle) == nullptr || id != capteurs.value(cle)->idCourant && cle != -1) {
            /* Si l'appareil est un FineOffset */
            if(TrouverCapteur(_trameRecue.object()["model"].toString()) == FINEOFFSET)
                capteurs.insert(cle, new WS2300(donnees,infosCapteur, *bd, *diffusion));
            else
                /* Ajoute un capteur Thermo/Hygro à la liste avec la clé défini précédemment */
                capteurs.insert(cle,new CapteursThermoHygro(donnees,infosCapteur, *bd, *diffusion));
        } else {
            /* Ajoute les données au capteur attribué à la clé */
            capteurs.value(cle)->AjouterDonnees(donnees);
        }

        /* Définie les valeurs permmettant la diffusion et l'enregistremment des données à true */
        capteurs.value(cle)->donneesAjoutees = true;
        capteurs.value(cle)->donneesADiffuser = true;

        /* Parcours la liste des capteurs */
        for (auto it = capteurs.begin(); it != capteurs.end(); ++it) {
            /* Si le capteur est créé et qu'il y a des données à diffuser */
            if(it.value() != nullptr && it.value()->donneesADiffuser == true) {
                /* Diffuse les données et remet la valeurs donneesADiffuser à false */
                it.value()->DiffuserDonnees(it.key(), donnees);
                it.value()->donneesADiffuser = false;
            }
        }
    }
}

/**
 * @brief RecepteurMeteo::TrouverCapteur
 * @param _capteur Modèle du capteur don les données viennent d'être reçues
 * @return Un TypeCapteurs permettant d'identifier le capteur
 * @details Identifie le type de capteur en fonction du modèle fournie en paramètre
 */
RecepteurMeteo::TypeCapteurs RecepteurMeteo::TrouverCapteur(const QString _capteur)
{
    if (_capteur == "Nexus-TH")
        return NEXUS;

    if (_capteur == "Oregon-THGR810")
        return OREGON;

    if (_capteur == "Fineoffset-WHx080")
        return FINEOFFSET;

    return AUTRE;
}

/**
 * @brief RecepteurMeteo::readOutput
 * @details Récupère les informations reçues du capteur en temps réel et les transforme dans un format
 * adapté à la bonne lecture du logiciel
 */
void RecepteurMeteo::readOutput()
{
    qDebug() << "------------------------------ Lecture de la sortie ------------------------------";
    QByteArray buffer;
    QByteArray donneesBrutes;

    /* Récupère les données fournies par le processus (RTL_433) */
    buffer = process->readAllStandardOutput();
    static QByteArray donneesAccumulees;
    donneesAccumulees.append(buffer);
    int index = donneesAccumulees.indexOf('}');

    if (index != -1) {
        // Extraire les données avant le caractère '}'
        donneesBrutes = donneesAccumulees.left(index + 1);

        // Traiter ou afficher les données
        qDebug() << "Données reçues : " << donneesBrutes;

        // Réinitialiser le buffer pour la prochaine lecture
        donneesAccumulees.clear();
    }

    /* Transforme les données en QJSonDocument */
    QJsonDocument jsonDoc = QJsonDocument::fromJson(donneesBrutes);
    TraiterTrame(jsonDoc);
}

/**
 * @brief RecepteurMeteo::onStarted
 * @details Valide le démarrage du programme RTL_433
 */
void RecepteurMeteo::onStarted()
{
    qDebug() << "Le processus rtl_433 a démarré.";
}

/**
 * @brief RecepteurMeteo::onFinished
 * @param exitCode Code erreur permettant l'identification de la raison de l'arrêt
 * @param exitStatus Statut du programme identifiant la raison de son arrêt
 * @details Informe de l'arrêt du programme RTL_433
 */
void RecepteurMeteo::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Le processus rtl_433 est terminé avec le code de sortie" << exitCode;

    if (exitStatus == QProcess::CrashExit) {
        qDebug() << "Le processus a échoué.";
    }
}

/**
 * @brief RecepteurMeteo::onTimer_timeout
 * @details Fonction lancé à la fin du timer, enregistre les données et relance le timer
 */
void RecepteurMeteo::onTimer_timeout()
{
    qDebug() << "------------------------------ Enregistrement des données ------------------------------";
    /* Parcours la liste des capteurs */
    for (auto it = capteurs.begin(); it != capteurs.end(); ++it) {
        /* Si le capteur est valide et qu'il y a eu de nouvelles données */
        if(it.value() != nullptr && it.value()->donneesAjoutees == true) {
            /* Demande l'enregistrement des données et remet l'attribut donneesAjoutees à false */
            it.value()->EnregistrerDonnees();
            it.value()->donneesAjoutees = false;
        }
    }

    /* Relance le timer */
    CalculerDelaiTimer();
}

/**
 * @brief RecepteurMeteo::CalculerDelaiTimer
 * @details Calcul et démarre le timer en fonction de l'heure actuelle, le timer doit
 * se lancer toutes les demi-heures
 */
void RecepteurMeteo::CalculerDelaiTimer()
{
    /* Récupère l'heure actuelle */
    QTime currentTime = QTime::currentTime();

    /* Attributs des valeurs pour les minutes, les secondes actuelles et les secondes avant le prochain enregistremment */
    int currentMinute = currentTime.minute();
    int currentSecond = currentTime.second();
    int secondsToNextTrigger = 0;

    /* Calcul le temps avant le prochain enregistrement */
    if (currentMinute < 30) {
        // Si c'est avant la demi-heure (par exemple 12h15, il faut attendre 15 minutes)
        secondsToNextTrigger = (30 - currentMinute) * 60 - currentSecond;
    } else {
        // Si c'est après (par exemple 12h45, il faut attendre 15 minutes pour le déclenchement suivant)
        secondsToNextTrigger = (60 - currentMinute) * 60 - currentSecond;
    }

    /* Transforme le temps avant le prochaine enregistrmement en ms */
    int delayInMs = secondsToNextTrigger * 1000;

    qDebug() << "Envoie dans : " << secondsToNextTrigger << "sec, soit " << secondsToNextTrigger/ 60 << "min";
    qDebug() << "A partir de : " << currentTime;

    /* Lance le timer */
    timer->start(delayInMs);
}
