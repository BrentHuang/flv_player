#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include <QDialog>

namespace Ui
{
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget* parent = nullptr);
    ~OptionsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::OptionsDialog* ui;
};

#endif // OPTIONS_DIALOG_H
