#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "signal_center.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    video_widget_.reset(new VideoWidget());
    this->setCentralWidget(video_widget_.get());

    StartThreads();
}

MainWindow::~MainWindow()
{
    StopThreads(); // 停止所有子线程
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (QMessageBox::Yes == QMessageBox::question(this, tr("Exit"), tr("Do you want to quit?")))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::StartThreads()
{
    file_parsers_ = new FileParsers();
    file_parsers_->moveToThread(&file_parse_thread_);
    connect(&file_parse_thread_, SIGNAL(finished()), file_parsers_, SLOT(deleteLater()));

    audio_decoders_ = new AudioDecoders();
    audio_decoders_->moveToThread(&audio_decode_thread_);
    connect(&audio_decode_thread_, SIGNAL(finished()), audio_decoders_, SLOT(deleteLater()));

    video_decoders_ = new VideoDecoders();
    video_decoders_->moveToThread(&video_decode_thread_);
    connect(&video_decode_thread_, SIGNAL(finished()), video_decoders_, SLOT(deleteLater()));

    pcm_player_ = new PcmPlayer();
    pcm_player_->moveToThread(&pcm_play_thread_);
    connect(&pcm_play_thread_, SIGNAL(finished()), pcm_player_, SLOT(deleteLater()));

    // 建立线程间通信的信号和槽
    connect(SIGNAL_CENTER, SIGNAL(FlvFileOpen(QString)), file_parsers_, SLOT(OnFlvFileOpen(QString)));
    connect(SIGNAL_CENTER, &SignalCenter::FlvH264TagReady, video_decoders_, &VideoDecoders::OnFlvH264TagReady);
    connect(SIGNAL_CENTER, &SignalCenter::Yuv420pReady, &yuv420p_player_, &Yuv420pPlayer::OnYuv420pReady);
    connect(SIGNAL_CENTER, &SignalCenter::Yuv420pPlay, video_widget_.get(), &VideoWidget::OnYuv420pPlay);
    connect(SIGNAL_CENTER, &SignalCenter::FlvAacTagReady, audio_decoders_, &AudioDecoders::OnFlvAacTagReady);
    connect(SIGNAL_CENTER, &SignalCenter::PcmReady, pcm_player_, &PcmPlayer::OnPcmReady);

    file_parse_thread_.start();
    audio_decode_thread_.start();
    video_decode_thread_.start();
    pcm_play_thread_.start();
}

void MainWindow::StopThreads()
{
    // disconnect线程间通信的信号和槽
    disconnect(SIGNAL_CENTER, SIGNAL(FlvFileOpen(QString)), file_parsers_, SLOT(OnFlvFileOpen(QString)));
    disconnect(SIGNAL_CENTER, &SignalCenter::FlvH264TagReady, video_decoders_, &VideoDecoders::OnFlvH264TagReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::Yuv420pReady, &yuv420p_player_, &Yuv420pPlayer::OnYuv420pReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::Yuv420pPlay, video_widget_.get(), &VideoWidget::OnYuv420pPlay);
    disconnect(SIGNAL_CENTER, &SignalCenter::FlvAacTagReady, audio_decoders_, &AudioDecoders::OnFlvAacTagReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::PcmReady, pcm_player_, &PcmPlayer::OnPcmReady);

    file_parse_thread_.quit();
    file_parse_thread_.wait();

    audio_decode_thread_.quit();
    audio_decode_thread_.wait();

    video_decode_thread_.quit();
    video_decode_thread_.wait();

    pcm_play_thread_.quit();
    pcm_play_thread_.wait();
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("Open file"), QString(), "flv files(*.flv)");
    if (file_path.isEmpty())
    {
        return;
    }

    this->ui->statusBar->showMessage(file_path);
    emit SIGNAL_CENTER->FlvFileOpen(file_path);
}
