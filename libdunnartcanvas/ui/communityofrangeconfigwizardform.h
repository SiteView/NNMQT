#ifndef COMMUNITYOFRANGECONFIGWIZARDFORM_H
#define COMMUNITYOFRANGECONFIGWIZARDFORM_H

#include <QDialog>
#include "wizard.h"
#include <QKeyEvent>
#include "CommonUtils/commonutil.h"
#include <QTableWidgetItem>
namespace Ui {
    class CommunityOfRangeConfigWizardForm;
}

class CommunityOfRangeConfigWizardForm : public QDialog, public Wizard
{
    Q_OBJECT

public:
    explicit CommunityOfRangeConfigWizardForm(Wizard * w, QWidget *parent = 0);
    ~CommunityOfRangeConfigWizardForm();

    Wizard * wizard;
    QWidget *parent;

protected:
    void keyPressEvent(QKeyEvent *event);
private:
    Ui::CommunityOfRangeConfigWizardForm *ui;
    void SaveTempCfg();
    CommonUtil* commonUtil;
private slots:
    void on_BtnLoad_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_addRow_DoubleClicked(int r,int c);
    void on_tableWidget_itemChanged(QTableWidgetItem *);
};

#endif // COMMUNITYOFRANGECONFIGWIZARDFORM_H
