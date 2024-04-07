#-------------------------------------------------
#
# Project created by QtCreator 2023-12-11T11:53:01
#
#-------------------------------------------------

HEADERS = \
    glwindow.h \
    ray.h \
    element.h

SOURCES = \
    glwindow.cpp \
    main.cpp \
    ray.cpp \
    element.cpp


TARGET = final_work

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    background.qrc

DEFINES += QT_DEPRECATED_WARNINGS
