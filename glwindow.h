#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWindow>
#include <QMatrix4x4>
#include <QVector3D>
#include "element.h"
#include "ray.h"

QT_BEGIN_NAMESPACE

class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;

QT_END_NAMESPACE

class GLWindow : public QOpenGLWindow
{
    Q_OBJECT
    Q_PROPERTY(float zValue READ zValue WRITE setZValue)
    Q_PROPERTY(float rotation READ rotation WRITE setRotation)
    Q_PROPERTY(float rotation2 READ rotation2 WRITE setRotation2)

public:
    GLWindow();
    ~GLWindow();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    float zValue() const { return m_eye.z(); }
    void setZValue(float v);

    float rotation() const { return m_rotation; }
    void setRotation(float v);

    float rotation2() const { return m_rotation2; }
    void setRotation2(float v);

    void keyPressEvent(QKeyEvent *event);

    QColor trace(Ray ray, int depth);

private slots:
    void startSecondStage();
    void changeBackground();

private:
    QOpenGLTexture *m_texture;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer *m_vbo;
    QOpenGLVertexArrayObject *m_vao;
    Element m_logo;
    int m_projMatrixLoc;
    int m_camMatrixLoc;
    int m_worldMatrixLoc;
    int m_myMatrixLoc;
    int m_lightPosLoc;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_world;
    QVector3D m_eye;
    QVector3D m_target;
    bool m_uniformsDirty;
    float m_rotation;
    float m_rotation2;
    QTimer* m_timer;
    int count;
    int lz;

    Help* m_scn;
};


#endif
