#include "glwindow.h"
#include <QImage>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLExtraFunctions>
#include <QPropertyAnimation>
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QKeyEvent>
#include <cmath>

#define Rg 60

GLWindow::GLWindow()
    : m_texture(nullptr),
      m_program(nullptr),
      m_vbo(nullptr),
      m_vao(nullptr),
      m_target(0, 0, -1),
      m_uniformsDirty(true),
      m_rotation(0),
      m_rotation2(0)
{
    m_world.setToIdentity();
    m_world.translate(0, 0, -1);
    m_world.rotate(180, 1, 0, 0);

    QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);
    animGroup->setLoopCount(-1);

    QPropertyAnimation *zAnim0 = new QPropertyAnimation(this, QByteArrayLiteral("zValue"));
    zAnim0->setStartValue(1.5f);
    zAnim0->setEndValue(10.0f);
    zAnim0->setDuration(2000);
    animGroup->addAnimation(zAnim0);

    QPropertyAnimation *zAnim1 = new QPropertyAnimation(this, QByteArrayLiteral("zValue"));
    zAnim1->setStartValue(10.0f);
    zAnim1->setEndValue(50.0f);
    zAnim1->setDuration(4000);
    zAnim1->setEasingCurve(QEasingCurve::OutElastic);
    animGroup->addAnimation(zAnim1);

    QPropertyAnimation *zAnim2 = new QPropertyAnimation(this, QByteArrayLiteral("zValue"));
    zAnim2->setStartValue(50.0f);
    zAnim2->setEndValue(1.5f);
    zAnim2->setDuration(2000);
    animGroup->addAnimation(zAnim2);

    animGroup->start();

    QPropertyAnimation* rotationAnim = new QPropertyAnimation(this, QByteArrayLiteral("rotation"));
    rotationAnim->setStartValue(0.0f);
    rotationAnim->setEndValue(360.0f);
    rotationAnim->setDuration(2000);
    rotationAnim->setLoopCount(-1);
    rotationAnim->start();

    QTimer::singleShot(4000, this, &GLWindow::startSecondStage);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GLWindow::changeBackground);
    m_timer->start(20000);
    count = 0;
    lz = Rg;
}

GLWindow::~GLWindow()
{
    makeCurrent();
    delete m_texture;
    delete m_program;
    delete m_vbo;
    delete m_vao;
    delete m_timer;
}

void GLWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_1) {
        changeBackground();
    }
    else if(event->key() == Qt::Key_2){
        lz = (lz+5)%Rg;
        qDebug("lz=%d",lz-Rg/2);
        m_uniformsDirty = true;
        update();
    }
}

void GLWindow::startSecondStage()
{
    QPropertyAnimation* rotation2Anim = new QPropertyAnimation(this, QByteArrayLiteral("rotation2"));
    rotation2Anim->setStartValue(0.0f);
    rotation2Anim->setEndValue(360.0f);
    rotation2Anim->setDuration(20000);
    rotation2Anim->setLoopCount(-1);
    rotation2Anim->start();
}

void GLWindow::changeBackground()
{
    count = (count+1) % 5;
    qDebug("Img %d",count);
    QImage img(QString(":/%1.png").arg(count));
    Q_ASSERT(!img.isNull());
    m_texture = new QOpenGLTexture(img.scaled(32, 36).mirrored());
    update();
}

void GLWindow::setZValue(float v)
{
    m_eye.setZ(v);
    m_uniformsDirty = true;
    update();
}

void GLWindow::setRotation(float v)
{
    m_rotation = v;
    m_uniformsDirty = true;
    update();
}

void GLWindow::setRotation2(float v)
{
    m_rotation2 = v;
    m_uniformsDirty = true;
    update();
}

static const char *vertexShader =
    "layout(location = 0) in vec4 vertex;\n"
    "layout(location = 1) in vec3 normal;\n"
    "out vec3 vert,vertNormal,color;\n"
    "uniform mat4 projMatrix,camMatrix,worldMatrix,myMatrix;\n"
    "uniform sampler2D sampler;\n"
    "void main() {\n"
    "   ivec2 pos = ivec2(gl_InstanceID % 32, gl_InstanceID / 32);\n"
    "   vec2 t = vec2(float(-16 + pos.x) * 0.8, float(-18 + pos.y) * 0.6);\n"
    "   float val = 2.0 * length(texelFetch(sampler, pos, 0).rgb);\n"
    "   mat4 wm = myMatrix * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, t.x, t.y, val, 1) * worldMatrix;\n"
    "   color = texelFetch(sampler, pos, 0).rgb * vec3(0.6, 0.8, 0.8);\n"
    "   vert = vec3(wm * vertex);\n"
    "   vertNormal = mat3(transpose(inverse(wm))) * normal;\n"
    "   gl_Position = projMatrix * camMatrix * wm * vertex;\n"
    "}\n";

static const char *fragmentShader =
    "in highp vec3 vert,vertNormal,color;\n"
    "out highp vec4 fragColor;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   fragColor = vec4(col, 1.0);\n"
    "}\n";

QByteArray versionedShaderCode(const char *src)
{
    QByteArray versionedSrc;

    if (QOpenGLContext::currentContext()->isOpenGLES())
        versionedSrc.append(QByteArrayLiteral("#version 300 es\n"));
    else
        versionedSrc.append(QByteArrayLiteral("#version 330\n"));

    versionedSrc.append(src);
    return versionedSrc;
}

QColor GLWindow::trace(Ray ray, int depth)
{
    float distance = INFINITY;
    int dep = depth;
    Object* aim = nullptr;
    QVector3D point, n, l, v;
    QColor hit_color = m_scn->get_ambient_light();
    int res_hit = 0;

    for (int k = 0; k < m_scn->get_object_count(); k++)
    {
        Object* obj = m_scn->get_object(k);
        int res;
        if (res = obj->is_intersected(ray, distance))
        {
            aim = obj;
            res_hit = res;
        }
    }

    if (distance != INFINITY)
    {
        point = ray.get_point(distance);
        n = aim->get_normal(point);
        n.normalize();

        if (res_hit == INTERSECTED_IN)
        {
            n = -n;
        }

        v = m_eye - point;
        v.normalize();

        for (int k = 0; k < m_scn->get_light_count(); k++)
        {
            PointLight pl = m_scn->get_light(k);
            l = pl.get_position() - point;
            l.normalize();

            Ray l_ray = Ray(point + l * SMALL, l);

            float shade = 1.0f;
            float distance = INFINITY;

            for (int k = 0; k < m_scn->get_object_count(); k++)
            {
                Object* obj = m_scn->get_object(k);
                if (obj->is_intersected(l_ray, distance))
                {
                    shade = 0.0f;
                    break;
                }
            }

            if (aim->get_diffuse() > 0)
            {
                float cos = QVector3D::dotProduct(l, n);
                if (cos > 0)
                {
                    float diffuse = cos * aim->get_diffuse() * shade;
                    diffuse++;
//                    hit_color = hit_color + diffuse * pl.get_color() * aim->get_color(point);
                }
            }

            if (aim->get_spec() > 0)
            {
                QVector3D h = 2 * QVector3D::dotProduct(n, l) * n - l;
                float cos = QVector3D::dotProduct(h, v);
                if (cos > 0)
                {
                    float specular = powf(cos, 20) * aim->get_spec() * shade;
                    specular++;
//                    hit_color = hit_color + specular * pl.get_color();
                }
            }
        }

        if (aim->get_refl() > 0 && dep < TOTALDEPTH)
        {
            QVector3D refl = 2 * QVector3D::dotProduct(n, l) * n - l;
            refl = 2*refl;
//            hit_color = hit_color + trace(Ray(point + refl * 0.0001f, refl), ++dep);
        }
    }
    return hit_color;
}

void GLWindow::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    QImage img(QString(":/%1.png").arg(count));
    Q_ASSERT(!img.isNull());
    m_texture = new QOpenGLTexture(img.scaled(32, 36).mirrored());

    if (m_program) {
        delete m_program;
        m_program = nullptr;
    }
    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, versionedShaderCode(vertexShader));
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, versionedShaderCode(fragmentShader));
    m_program->link();

    m_projMatrixLoc = m_program->uniformLocation("projMatrix");
    m_camMatrixLoc = m_program->uniformLocation("camMatrix");
    m_worldMatrixLoc = m_program->uniformLocation("worldMatrix");
    m_myMatrixLoc = m_program->uniformLocation("myMatrix");
    m_lightPosLoc = m_program->uniformLocation("lightPos");

    if (m_vao) {
        delete m_vao;
        m_vao = nullptr;
    }
    m_vao = new QOpenGLVertexArrayObject;
    if (m_vao->create()) m_vao->bind();

    if (m_vbo) {
        delete m_vbo;
        m_vbo = nullptr;
    }
    m_program->bind();
    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(m_logo.constData(), m_logo.count() * sizeof(GLfloat));
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    m_vbo->release();

    f->glEnable(GL_DEPTH_TEST);
    f->glEnable(GL_CULL_FACE);
}

void GLWindow::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
    m_uniformsDirty = true;
}

void GLWindow::paintGL()
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    f->glClearColor(0.2f, 0.2f, 0.2f, 1);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();
    m_texture->bind();

    if (m_uniformsDirty) {
        m_uniformsDirty = false;
        QMatrix4x4 camera;
        camera.lookAt(m_eye, m_eye + m_target, QVector3D(0, 1, 0));
        m_program->setUniformValue(m_projMatrixLoc, m_proj);
        m_program->setUniformValue(m_camMatrixLoc, camera);
        QMatrix4x4 wm = m_world;
        wm.rotate(m_rotation, 1, 1, 0);
        m_program->setUniformValue(m_worldMatrixLoc, wm);
        QMatrix4x4 mm;
        mm.setToIdentity();
        mm.rotate(-m_rotation2, 1, 0, 0);
        m_program->setUniformValue(m_myMatrixLoc, mm);
        m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, lz-Rg/2));
    }

    f->glDrawArraysInstanced(GL_TRIANGLES, 0, m_logo.vertexCount(), 32 * 36);
}
