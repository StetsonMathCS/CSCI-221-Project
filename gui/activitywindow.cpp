#include "gui/activitywindow.h"
#include "ui_activitywindow.h"
#include "gui/checkinwindow.h"
#include "gui/listactivities.h"
#include <QString>
#include "QRCapture.h"

ActivityWindow::ActivityWindow(QWidget *parent, Activity* act) :
    QDialog(parent),
    ui(new Ui::ActivityWindow)
{
    ui->setupUi(this);
    activity=act;
    QString name = QString::fromStdString(activity->getActivityName());
    ui->label_2->setText(name);
}

ActivityWindow::~ActivityWindow()
{
    delete ui;
}

void ActivityWindow::on_QR_released()
{
    Camera scanner(this, activity);
    this->hide();
    scanner.setModal(true);
    scanner.exec();
    this->show();
}


void ActivityWindow::on_back_released()
{
    ListActivities la;
    this->hide();
    la.setModal(true);
    la.exec();
    this->show();
}
