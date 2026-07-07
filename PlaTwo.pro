QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    core/authmanager.h \
    core/storagemanager.h \
    models/user.h \
    network/gameclient.h \
    network/gamemessage.h \
    network/gameroom.h \
    ui/gamelobbywindow.h \
    ui/loginwindow.h \
    # ui/mainmenuwindow.h \
    ui/mainmenuwindow.h \
    ui/recoverywindow.h \
    ui/signupwindow.h

SOURCES += \
    core/authmanager.cpp \
    core/storagemanager.cpp \
    main.cpp \
    network/gameclient.cpp \
    ui/gamelobbywindow.cpp \
    ui/loginwindow.cpp \
    # ui/mainmenuwindow.cpp \
    ui/mainmenuwindow.cpp \
    ui/recoverywindow.cpp \
    ui/signupwindow.cpp

FORMS += \
    ui/gamelobbywindow.ui \
    ui/loginwindow.ui \
    ui/mainmenuwindow.ui \
    ui/recoverywindow.ui \
    ui/signupwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
