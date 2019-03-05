#include "options_dialog.h"
#include "ui_options_dialog.h"
#include "global.h"

OptionsDialog::OptionsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::MSWindowsFixedSizeDialogHint);
    ui->radioButton->setChecked(AUDIO_DECODER_FDKAAC == GLOBAL->config.GetAudioDecoderId());
    ui->radioButton_2->setChecked(AUDIO_DECODER_FFMPEG == GLOBAL->config.GetAudioDecoderId());
    ui->radioButton_3->setChecked(VIDEO_DECODER_OPENH264 == GLOBAL->config.GetVideoDecoderId());
    ui->radioButton_4->setChecked(VIDEO_DECODER_FFMPEG == GLOBAL->config.GetVideoDecoderId());
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_accepted()
{
    if (ui->radioButton->isChecked())
    {
        GLOBAL->config.SetAudioDecoderId(AUDIO_DECODER_FDKAAC);
    }
    else
    {
        GLOBAL->config.SetAudioDecoderId(AUDIO_DECODER_FFMPEG);
    }

    if (ui->radioButton_3->isChecked())
    {
        GLOBAL->config.SetVideoDecoderId(VIDEO_DECODER_OPENH264);
    }
    else
    {
        GLOBAL->config.SetVideoDecoderId(VIDEO_DECODER_FFMPEG);
    }
}
