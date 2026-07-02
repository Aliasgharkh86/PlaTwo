QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/authmanager.cpp \
    core/storagemanager.cpp \
    main.cpp \
    mainwindow.cpp \
<<<<<<< HEAD
    ui/loginwindow.cpp

HEADERS += \
    core/authmanager.h \
    core/storagemanager.h \
    mainwindow.h \
    models/user.h
=======
    ui/loginwindow.cpp \
    ui/signupwindow.cpp \
    ui/recoverywindow.cpp

HEADERS += \
    core/authmanager.h \
    mainwindow.h \
    ui/loginwindow.h \
    ui/signupwindow.h \
    ui/recoverywindow.h
>>>>>>> ddb5dbe5f15c3ce900079096e5f7dc06cf9a6142

FORMS += \
    mainwindow.ui \
    ui/loginwindow.ui \
    ui/signupwindow.ui \
    ui/recoverywindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
