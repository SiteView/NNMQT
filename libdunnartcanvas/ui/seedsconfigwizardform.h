#ifndef SEEDSCONFIGWIZARDFORM_H
#define SEEDSCONFIGWIZARDFORM_H

#include <QDialog>
#include "wizard.h"
#include <QList>
#include <QString>
#include <QKeyEvent>
#include <QTableWidgetItem>
namespace Ui {
    class SeedsConfigWizardForm;
}

class SeedsConfigWizardForm : public QDialog
{
    Q_OBJECT

public:
    explicit SeedsConfigWizardForm(Wizard * w, QWidget *parent = 0);
    ~SeedsConfigWizardForm();

    Wizard * wizard;
    QWidget *parent;

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::SeedsConfigWizardForm *ui;
    void SaveTempCfg();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_addRow_DoubleClicked(int r,int c);
    void on_tableWidget_itemChanged(QTableWidgetItem *);
};

#endif // SEEDSCONFIGWIZARDFORM_H
