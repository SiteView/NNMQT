#ifndef SCANPERFORMANCECONFIGWIZARDFORM_H
#define SCANPERFORMANCECONFIGWIZARDFORM_H

#include <QDialog>
#include "wizard.h"

namespace Ui {
    class ScanPerformanceConfigWizardForm;
}

class ScanPerformanceConfigWizardForm : public QDialog
{
    Q_OBJECT

public:
    explicit ScanPerformanceConfigWizardForm(Wizard * w, QWidget *parent = 0);
    ~ScanPerformanceConfigWizardForm();

    Wizard * wizard;
    QWidget *parent;
private:
    Ui::ScanPerformanceConfigWizardForm *ui;
    void SaveTempCfg();

    void Initialize(Wizard * w);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_3_clicked();
};

#endif // SCANPERFORMANCECONFIGWIZARDFORM_H
