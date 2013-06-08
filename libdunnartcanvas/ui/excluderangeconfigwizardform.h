#ifndef EXCLUDERANGECONFIGWIZARDFORM_H
#define EXCLUDERANGECONFIGWIZARDFORM_H

#include <QDialog>
#include "wizard.h"
#include "CommonUtils/commonutil.h"
#include <QKeyEvent>
#include <QTableWidgetItem>
namespace Ui {
    class ExcludeRangeConfigWizardForm;
}

class ExcludeRangeConfigWizardForm : public QDialog
{
    Q_OBJECT

public:
    explicit ExcludeRangeConfigWizardForm(Wizard * w, QWidget *parent = 0);
    ~ExcludeRangeConfigWizardForm();

    Wizard * wizard;
    QWidget *parent;

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::ExcludeRangeConfigWizardForm *ui;
    void SaveTempCfg();
    CommonUtil* commonUtil;
    bool isFirst;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_addRow_DoubleClicked(int r,int c);
    void on_tableWidget_itemChanged(QTableWidgetItem *);
};

#endif // EXCLUDERANGECONFIGWIZARDFORM_H
