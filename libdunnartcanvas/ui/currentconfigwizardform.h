#ifndef CURRENTCONFIGWIZARDFORM_H
#define CURRENTCONFIGWIZARDFORM_H

#include <QDialog>
#include "wizard.h"

namespace Ui {
    class CurrentConfigWizardForm;
}

class CurrentConfigWizardForm : public QDialog
{
    Q_OBJECT

public:
    explicit CurrentConfigWizardForm(Wizard * w, QWidget *parent = 0);
    ~CurrentConfigWizardForm();

    Wizard * wizard;
    QWidget *parent;
private:
    Ui::CurrentConfigWizardForm *ui;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
};

#endif // CURRENTCONFIGWIZARDFORM_H
