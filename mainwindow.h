#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QThread>
#include "video_widget.h"
#include "pcm_player.h"
#include "yuv420p_player.h"
#include "file/file_parsers.h"
#include "audio/audio_decoders.h"
#include "video/video_decoders.h"

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

private slots:
    void on_actionOpen_triggered();

private:
    void StartThreads();
    void StopThreads();

private:
    Ui::MainWindow* ui;

    std::unique_ptr<VideoWidget> video_widget_;

    FileParsers* file_parsers_;
    QThread file_parse_thread_;

    AudioDecoders* audio_decoders_;
    QThread audio_decode_thread_;

    VideoDecoders* video_decoders_;
    QThread video_decode_thread_;

    PcmPlayer* pcm_player_;
    QThread pcm_play_thread_;

    Yuv420pPlayer yuv420p_player_;
};

#endif // MAINWINDOW_H
