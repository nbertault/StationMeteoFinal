#ifndef INTERFACETEST_H
#define INTERFACETEST_H

#include <QDialog>
#include <QApplication>
#include "ws2300.h"
#include <QTimer>
#include "capteursthermohygro.h"
#include "diffuseurmeteo.h"
#include "accesbdd.h"

namespace Ui {
class InterfaceTest;
}

class InterfaceTest : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceTest(QWidget *parent = nullptr);
    ~InterfaceTest();

private slots:


    void on_comboBoxModele_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::InterfaceTest *ui;
    QString idCourant, etatBatterie, canal, modele;
    WS2300 *ws2300;
    CapteursThermoHygro *Nexus;
    QTimer *timer;
    DiffuseurMeteo *diffusion;
};

#endif // INTERFACETEST_H
