#include "gui/listactivities.h"
#include "ui_listactivities.h"
#include "gui/activitysearch.h"
#include "gui/activitywindow.h"


ListActivities::ListActivities(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ListActivities)
{
    ui->setupUi(this);
    for (int t = 0; t<100;t++)
    {
        ui->listWidget->addItem("test " + QString::number(i));

        i++;
    }
}

ListActivities::~ListActivities()
{
    delete ui;
}

void ListActivities::on_pushButton_released()
{
    ActivitySearch la;
    this->hide();
    la.setModal(true);
    la.exec();
    this->show();
}

void ListActivities::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ActivityWindow la;
    this->hide();
    la.setModal(true);
    la.exec();
    this->show();
}

void ListActivities::on_pushButton_2_released()
{
    this->close();
}