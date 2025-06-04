#include <QCoreApplication>
#include "interfacetest.h"
#include "recepteurmeteo.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RecepteurMeteo recepteurMeteo;
    // InterfaceTest uneInterface;
    // uneInterface.show();
    return a.exec();
}
