# StationMeteoFinal

## 📌 Présentation

Ce projet consiste à développer une **station météorologique connectée** capable de **recevoir, stocker et diffuser en temps réel** des données environnementales (température, humidité, pression, vent, précipitations) à partir de capteurs sans fil 433 MHz. Les données sont ensuite accessibles via une **application Android**, une **interface web**, et peuvent être consultées à distance.

Le système repose sur un **Raspberry Pi**, une **clé RTL-SDR**, l’outil **rtl_433**, une base de données **MariaDB**, et une application développée en **Qt avec WebSockets**.

## ⚙️ Fonctionnalités

- Réception des données météorologiques via **RTL_433**.
- Analyse et conversion des trames en **JSON**.
- Stockage des données dans une base **MariaDB**.
- Diffusion en **temps réel** via **WebSockets**.
- Accès multiplateforme : client **Android**, **interface web**, et **accès distant**.
- Interface de développement simple pour les tests et la supervision.

## 🧱 Architecture

- **Langage principal** : C++ (Qt)
- **Base de données** : MariaDB
- **Communication temps réel** : WebSockets
- **Traitement JSON** : QJsonObject
- **Réception radio** : rtl_433 + clé RTL-SDR
- **Serveur** : Raspberry Pi
- **Client** : Android, Web, Client distant

## 📁 Structure du dépôt

```bash
📦 StationMeteoFinal/
├── [src]                  # Code source Qt (C++)
│   ├── [main.cpp]
│   ├── [websocketserver.cpp]
│   ├── [sensorparser.cpp]
│   └── [databasehandler.cpp]
├── [scripts]          # Scripts de lancement/configuration
│   └── [start_rtl433.sh]
├── [android]          # Application mobile Android
├── [web]                  # Interface web de consultation
├── [sql]                  # Scripts de base de données
│   └── [schema.sql]
├── [doc]                  # Documentation technique
│   └── [fiche_test_bdd.pdf]
├── [README.md]     # Présentation du projet
└── [.gitignore]    # Fichiers à ignorer par Git
