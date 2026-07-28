QT += widgets network

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

HEADERS += \
    core/authmanager.h \
    core/storagemanager.h \
    games/dotsandboxesgame.h \
    games/fanoronagame.h \
    games/game.h \
    games/ninemensmorrisgame.h \
    models/gamerecord.h \
    models/user.h \
    network/gameclient.h \
    network/gamemessage.h \
    network/gameroom.h \
    network/gameserver.h \
    ui/chatwidget.h \
    ui/dotsandboxeswidget.h \
    ui/editprofilewindow.h \
    ui/fanoronagamewidget.h \
    ui/gameboardwindow.h \
    ui/gamelobbywindow.h \
    ui/historywindow.h \
    ui/loginwindow.h \
    ui/signupwindow.h \
    ui/mainmenuwindow.h \
    ui/ninemensmorriswidget.h \
    ui/recoverywindow.h

SOURCES += \
    core/authmanager.cpp \
    core/storagemanager.cpp \
    games/dotsandboxesgame.cpp \
    games/fanoronagame.cpp \
    games/ninemensmorrisgame.cpp \
    main.cpp \
    network/gameclient.cpp \
    network/gameserver.cpp \
    ui/chatwidget.cpp \
    ui/dotsandboxeswidget.cpp \
    ui/editprofilewindow.cpp \
    ui/fanoronagamewidget.cpp \
    ui/gameboardwindow.cpp \
    ui/gamelobbywindow.cpp \
    ui/historywindow.cpp \
    ui/loginwindow.cpp \
    ui/signupwindow.cpp \
    ui/mainmenuwindow.cpp \
    ui/ninemensmorriswidget.cpp \
    ui/recoverywindow.cpp

FORMS += \
    ui/chatwidget.ui \
    ui/editprofilewindow.ui \
    ui/gamelobbywindow.ui \
    ui/loginwindow.ui \
    ui/mainmenuwindow.ui \
    ui/recoverywindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target