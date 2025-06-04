#include "interfacetest.h"
#include "ui_interfacetest.h"

InterfaceTest::InterfaceTest(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InterfaceTest)
    ,ws2300(nullptr)
    ,Nexus(nullptr)
{
    ui->setupUi(this);
    diffusion = new DiffuseurMeteo;
}

InterfaceTest::~InterfaceTest()
{
    delete ui;
}


void InterfaceTest::on_comboBoxModele_currentTextChanged(const QString &arg1)
{
    if(arg1 == "FineOffset") {
        ui->groupBox_3->show();
        ui->groupBox_4->show();
    }
    if(arg1 == "Nexus") {
        ui->groupBox_3->hide();
        ui->groupBox_4->hide();
    }
}


void InterfaceTest::on_pushButton_clicked()
{
    QStringList infoCapteur;
    infoCapteur << ui->lineEditidCourant->text() << ui->lineEditCanal->text() << ui->comboBoxModele->currentText();

    if(ui->comboBoxModele->currentText() == "FineOffset") {
        QString donnees = ui->lineEditHumidite->text() + ',' + ui->lineEditTemperature->text() + ';' + ui->lineEditVitesseMoyenne->text() + ',' + ui->lineEditDirection->text() + ',' + ui->lineEditVitesseMaximum->text() + ';' + ui->lineEditPrecipitation->text();

        if (ws2300 == nullptr || ws2300->idCourant != ui->lineEditidCourant->text().toInt())
            // ws2300 = new WS2300(donnees, infoCapteur, laBDD);
            qDebug() << "------------------------------ Ajout FineOffset ------------------------------";

        else{
            qDebug() << "------------------------------ Ajout Données FineOffset ------------------------------";
            ws2300->AjouterDonnees(donnees);
        }
        ws2300->EnregistrerDonnees();
        diffusion->DiffuserTrame(4,donnees);
    }

    if (ui->comboBoxModele->currentText() == "Nexus") {
        QString donnees = ui->lineEditHumidite->text() + ',' + ui->lineEditTemperature->text();

        if (Nexus == nullptr || Nexus->idCourant != ui->lineEditidCourant->text().toInt())
            qDebug() << "------------------------------ Ajout Nexus ------------------------------";
        // Nexus = new CapteursThermoHygro(donnees, infoCapteur, );
        else{
            qDebug() << "------------------------------ Ajout Données Nexus ------------------------------";
            Nexus->AjouterDonnees(donnees);
        }
        Nexus->EnregistrerDonnees();
    }
}

