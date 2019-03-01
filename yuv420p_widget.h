#ifndef YUV420P_WIDGET_H
#define YUV420P_WIDGET_H

#include <memory>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include "yuv420p.h"

class Yuv420pWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    explicit Yuv420pWidget(QWidget* parent = nullptr);
    virtual ~Yuv420pWidget();

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

#endif // YUV420P_WIDGET_H
