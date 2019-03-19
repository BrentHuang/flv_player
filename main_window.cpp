#include "main_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_main_window.h"
#include "signal_center.h"
#include "global.h"
#include "options_dialog.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    yuv420p_render_ = new Yuv420pRender(this);
    setCentralWidget(yuv420p_render_);

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
        GLOBAL->app_exit = true;

        GLOBAL->file_parse_mutex.lock();
        GLOBAL->file_parse_cond.wakeAll();
        GLOBAL->file_parse_mutex.unlock();

        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::StartThreads()
{
    qDebug() << __FILE__ << ":" << __LINE__ << "MainWindow::StartThreads() " << QThread::currentThreadId();

    // TODO 可以把启动文件解析和解码线程放到打开文件的时候
    file_parsers_ = new FileParsers();
    file_parsers_->moveToThread(&file_parse_thread_);
    connect(&file_parse_thread_, SIGNAL(finished()), file_parsers_, SLOT(deleteLater()));

    audio_decoders_ = new AudioDecoders();
    audio_decoders_->moveToThread(&audio_decode_thread_);
    connect(&audio_decode_thread_, SIGNAL(finished()), audio_decoders_, SLOT(deleteLater()));

    video_decoders_ = new VideoDecoders();
    video_decoders_->moveToThread(&video_decode_thread_);
    connect(&video_decode_thread_, SIGNAL(finished()), video_decoders_, SLOT(deleteLater()));

    // 建立线程间通信的信号和槽
    connect(SIGNAL_CENTER, &SignalCenter::FlvFileOpen, file_parsers_, &FileParsers::OnFlvFileOpen);
    connect(SIGNAL_CENTER, &SignalCenter::FlvH264TagReady, video_decoders_, &VideoDecoders::OnFlvH264TagReady);
    connect(SIGNAL_CENTER, &SignalCenter::Yuv420pReady, &yuv420p_player_, &Yuv420pPlayer::OnYuv420pReady);
    connect(SIGNAL_CENTER, &SignalCenter::Yuv420pPlay, yuv420p_render_, &Yuv420pRender::OnYuv420pPlay);
    connect(SIGNAL_CENTER, &SignalCenter::FlvAacTagReady, audio_decoders_, &AudioDecoders::OnFlvAacTagReady);
    connect(SIGNAL_CENTER, &SignalCenter::PcmReady, &pcm_player_, &PcmPlayer::OnPcmReady);

    file_parse_thread_.start();
    audio_decode_thread_.start();
    video_decode_thread_.start();
}

void MainWindow::StopThreads()
{
    qDebug() << __FILE__ << ":" << __LINE__ << "MainWindow::StopThreads() " << QThread::currentThreadId();

    // disconnect线程间通信的信号和槽
    disconnect(SIGNAL_CENTER, &SignalCenter::FlvFileOpen, file_parsers_, &FileParsers::OnFlvFileOpen);
    disconnect(SIGNAL_CENTER, &SignalCenter::FlvH264TagReady, video_decoders_, &VideoDecoders::OnFlvH264TagReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::Yuv420pReady, &yuv420p_player_, &Yuv420pPlayer::OnYuv420pReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::Yuv420pPlay, yuv420p_render_, &Yuv420pRender::OnYuv420pPlay);
    disconnect(SIGNAL_CENTER, &SignalCenter::FlvAacTagReady, audio_decoders_, &AudioDecoders::OnFlvAacTagReady);
    disconnect(SIGNAL_CENTER, &SignalCenter::PcmReady, &pcm_player_, &PcmPlayer::OnPcmReady);

    file_parse_thread_.quit();
    file_parse_thread_.wait();

    audio_decode_thread_.quit();
    audio_decode_thread_.wait();

    video_decode_thread_.quit();
    video_decode_thread_.wait();
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("Open file"), QString(), "flv files(*.flv)");
    if (file_path.isEmpty())
    {
        return;
    }

    this->ui->actionOpen->setEnabled(false);
    this->ui->statusBar->showMessage(file_path);

    emit SIGNAL_CENTER->FlvFileOpen(file_path);
}

void MainWindow::on_actionOptions_triggered()
{
    OptionsDialog options_dialog;
    options_dialog.exec(); // show(): 非模态  open(): 半模态  exec(): 模态
}
