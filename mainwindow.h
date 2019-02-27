#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QCloseEvent>
#include "video_widget.h"
#include "yuv420p_player.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent* event) override;

private:
    void StartThreads();
    void StopThreads();

private:
    Ui::MainWindow* ui;

    std::unique_ptr<VideoWidget> video_widget_;
    QThread flv_parser_thread_;
    QThread aac_decode_thread_;
    QThread h264_decode_thread_;
    QThread pcm_play_thread_;
    Yuv420pPlayer yuv420p_player_;
};

#endif // MAINWINDOW_H
