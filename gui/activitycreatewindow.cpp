#include "gui/activitycreatewindow.h"
#include "ui_activitycreatewindow.h"
#include "database/activity.h"
#include "gui/prereqselectwindow.h"

ActivityCreateWindow::ActivityCreateWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActivityCreateWindow)
{
    ui->setupUi(this);
}

ActivityCreateWindow::~ActivityCreateWindow()
{
    delete ui;
}

void ActivityCreateWindow::on_cancelButton_released()
{
    this->close();
}

void ActivityCreateWindow::on_createActivityButton_released()
{
    /* This will take data in textfields
     * and then pass to controller
     * then close window
     */
    std::string name = ui->actNameTF->text().toStdString();
    std::string status;
   //get activity status
   if(ui->activeRadioButton->isChecked())
   {
       status="active";
   }
   if(ui->upcomingRadioButton->isChecked())
   {
       status="upcoming";
   }
   Activity::createActivity(name,1,status);
    this->close();

}

void ActivityCreateWindow::on_preReqSelectButton_released()
{

    //PrereqSelectWindow* psw = new PrereqSelectWindow(this, acts);
    PrereqSelectWindow psw;
    psw.setModal(true);
    psw.exec();


}
