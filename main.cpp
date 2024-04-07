#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QOpenGLContext>

#include "glwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);

    qDebug("Requesting 3.3 compatibility context");
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);

    QSurfaceFormat::setDefaultFormat(fmt);

    GLWindow glWindow;
    glWindow.showMaximized();

    return app.exec();
}
