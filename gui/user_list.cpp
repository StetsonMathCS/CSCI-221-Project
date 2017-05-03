#include "gui/user_list.h"
#include "ui_user_list.h"
#include "gui/mainwindow.h"
#include "gui/user_search.h"
#include "gui/user_view.h"
#include "gui/eventadminwindow.h"

user_list::user_list(QWidget *parent) :
    QDialog(parent),
    //ui(new Ui::user_list)
    ui(new Ui::user_list)
{
    ui->setupUi(this);

    for (int i = 0; i<51;i++)
    {
        ui->listWidget->addItem("test " + QString::number(i));
    }
}

user_list::~user_list()
{
    delete ui;
}

void user_list::on_pushButton_clicked()
{
   this->close();
}

//void user_list::on_pushButton_2_clicked()
//{
//    user_view userView;
//    this->hide();
//    userView.setModal(true);
//    userView.exec();
//    this->show();
//}

void user_list::on_pushButton_3_clicked()
{
    user_search userSearch;
    this->hide();
    userSearch.setModal(true);
    userSearch.exec();
    this->show();
}

void user_list::on_listWidget_itemClicked(QListWidgetItem *item)
{
    user_view userView;
    this->hide();
    userView.setModal(true);
    userView.exec();
    this->show();
}
