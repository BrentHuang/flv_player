#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include "yuv420p.h"

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    VideoWidget();
    virtual ~VideoWidget();

public slots:
    void OnFrameRender(Yuv420p* frame);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram program_;
    GLuint idy_, idu_, idv_;
    Yuv420p* frame_;
};

#endif // VIDEO_WIDGET_H
