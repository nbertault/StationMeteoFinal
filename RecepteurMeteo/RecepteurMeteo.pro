QT += gui sql core widgets websockets network

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        accesbdd.cpp \
        capteurs.cpp \
        capteursthermohygro.cpp \
        diffuseurmeteo.cpp \
        interfacetest.cpp \
        main.cpp \
        recepteurmeteo.cpp \
        ws2300.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    accesbdd.h \
    capteurs.h \
    capteursthermohygro.h \
    diffuseurmeteo.h \
    interfacetest.h \
    recepteurmeteo.h \
    ws2300.h

FORMS += \
    interfacetest.ui
