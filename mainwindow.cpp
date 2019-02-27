#include "mainwindow.h"
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "flv_parser.h"
#include "aac_decoder.h"
#include "h264_decoder.h"
#include "pcm_player.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    video_widget_.reset(new VideoWidget());
    this->setCentralWidget(video_widget_.get());
}

MainWindow::~MainWindow()
{
    StopThreads();

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
    FlvParser* flv_parser = new FlvParser();
    flv_parser->moveToThread(&flv_parser_thread_);
    connect(&flv_parser_thread_, SIGNAL(finished()), flv_parser, SLOT(deleteLater()));

    AacDecoder* aac_decoder = new AacDecoder();
    aac_decoder->moveToThread(&aac_decode_thread_);
    connect(&aac_decode_thread_, SIGNAL(finished()), aac_decoder, SLOT(deleteLater()));

    H264Decoder* h264_decoder = new H264Decoder();
    h264_decoder->moveToThread(&h264_decode_thread_);
    connect(&h264_decode_thread_, SIGNAL(finished()), h264_decoder, SLOT(deleteLater()));

    PcmPlayer* pcm_player = new PcmPlayer();
    pcm_player->moveToThread(&pcm_play_thread_);
    connect(&pcm_play_thread_, SIGNAL(finished()), pcm_player, SLOT(deleteLater()));

    // 建立线程间通信的信号和槽
//    connect(flv_parser, &FlvParser::AudioTagReady(), aac_decoder, &AacDecoder::DecodeAac());
//    connect(aac_decoder, &AacDecoder::PcmReady(), pcm_player, &PcmPlayer::PlayPcm());

//    connect(flv_parser, &FlvParser::VideoTagReady(), h264_decoder, &H264Decoder::DecodeH264());
//    connect(h264_decoder, &H264Decoder::Yuv420pReady(), yuv420p_player_, &Yuv420pPlayer::RecvYuv420p());

    flv_parser_thread_.start();
    aac_decode_thread_.start();
    h264_decode_thread_.start();
    pcm_play_thread_.start();
}

void MainWindow::StopThreads()
{
    flv_parser_thread_.quit();
    flv_parser_thread_.wait();

    aac_decode_thread_.quit();
    aac_decode_thread_.wait();

    h264_decode_thread_.quit();
    h264_decode_thread_.wait();

    pcm_play_thread_.quit();
    pcm_play_thread_.wait();
}
