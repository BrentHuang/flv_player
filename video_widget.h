#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <memory>
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
    void OnYuv420pPlay(std::shared_ptr<Yuv420p> yuv420p);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram program_;
    GLuint idy_, idu_, idv_;
    std::shared_ptr<Yuv420p> yuv420p_;
};

#endif // VIDEO_WIDGET_H
