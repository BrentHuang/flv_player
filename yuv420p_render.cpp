#include "yuv420p_render.h"
#include <QThread>

static const GLfloat vertex_vertices[] =
{
    -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };

static const GLfloat texture_vertices[] =
{
    0.0f,  1.0f,
    1.0f,  1.0f,
    0.0f,  0.0f,
    1.0f,  0.0f
};

Yuv420pRender::Yuv420pRender(QWidget* parent) : QOpenGLWidget(parent), yuv420p_()
{
}

Yuv420pRender::~Yuv420pRender()
{
}

void Yuv420pRender::OnYuv420pPlay(std::shared_ptr<Yuv420p> yuv420p)
{
    yuv420p_ = yuv420p;
    update();
}

void Yuv420pRender::initializeGL()
{
    initializeOpenGLFunctions();

    if (!program_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/yuv420p.vert"))
    {
        qDebug() << "add shader failed";
        return;
    }

    if (QOpenGLContext::currentContext()->isOpenGLES())
    {
        qDebug() << "opengl es";
        if (!program_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/yuv420p_es.frag"))
        {
            qDebug() << "add shader failed";
            return;
        }
    }
    else
    {
        qDebug() << "opengl desktop";
        if (!program_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/yuv420p_desktop.frag"))
        {
            qDebug() << "add shader failed";
            return;
        }
    }

    if (!program_.link())
    {
        qDebug() << "link shader failed";
        return;
    }

    if (!program_.bind())
    {
        qDebug() << "bind shader failed";
        return;
    }

    GLuint ids[3];
    glGenTextures(3, ids);
    idy_ = ids[0];
    idu_ = ids[1];
    idv_ = ids[2];

    program_.release();
}

void Yuv420pRender::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Yuv420pRender::paintGL()
{
//    qDebug() << "Yuv420pWidget::paintGL " << QThread::currentThreadId(); // 主线程中

    if (nullptr == yuv420p_)
    {
        return;
    }

    Yuv420p* yuv420p = yuv420p_.get();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program_.bind();

    program_.enableAttributeArray("vertexIn");
    program_.enableAttributeArray("textureIn");

    program_.setAttributeArray("vertexIn", GL_FLOAT, vertex_vertices, 2);
    program_.setAttributeArray("textureIn", GL_FLOAT, texture_vertices, 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, idy_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yuv420p->y.width, yuv420p->y.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuv420p->y.data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, idu_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yuv420p->u.width, yuv420p->u.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuv420p->u.data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, idv_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yuv420p->v.width, yuv420p->v.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuv420p->v.data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    program_.setUniformValue("textureY", 0);
    program_.setUniformValue("textureU", 1);
    program_.setUniformValue("textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    program_.disableAttributeArray("vertexIn");
    program_.disableAttributeArray("textureIn");

    program_.release();
    yuv420p_.reset();
}
