#include "showuser.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlquerymodel.h"
#include "ui_showuser.h"
#include <QMessageBox.h>

static QSqlQueryModel *mode;
static QSqlDatabase db;//数据库

showUser::showUser(QSqlDatabase dbt,QWidget *parent): QDialog(parent), ui(new Ui::showUser){
    ui->setupUi(this);
    db = dbt;
    mode = new QSqlQueryModel(ui->Output);
    mode->setQuery("select userid,favoritePlace from user",db);
    mode->setHeaderData(0,Qt::Horizontal,tr("用户名"));
    mode->setHeaderData(1, Qt::Horizontal, tr("最喜爱的地点"));
    ui->Output->setModel(mode);
    ui->Output->resizeRowsToContents();
    ui->Output->resizeColumnsToContents();
}

showUser::~showUser(){
    delete mode;
    delete ui;
}

void showUser::on_deleteSelected_clicked(){//删除选中
    QSqlQuery query(db);
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//获取用户id列
    //空选择
    if(data.size()<=0){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择行"), QMessageBox::tr("确定"));
        return;
    }
    //检查是否管理员账号，是则禁止删除
    if(data.at(0).data().toString()=="admin"){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("禁止删除管理员账号"), QMessageBox::tr("确定"));
        return;
    }
    //操作
    query.prepare("call DeleteUser(?)");
    query.addBindValue(data.at(0).data().toString());
    query.exec();
    mode->setQuery("select userid,favoritePlace from user",db);//更新
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除成功"), QMessageBox::tr("确定"));
    ui->userIDInput->clear();
}

void showUser::on_updateUserID_clicked(){//修改id(修改前需要删除用户数据)
    QSqlQuery query(db);
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//获取用户id列
    //空选择
    if(data.size()<=0){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择行"), QMessageBox::tr("确定"));
        return;
    }
    //检查输入
    if(ui->userIDInput->text().isEmpty()){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("用户名不能为空"), QMessageBox::tr("确定"));
        return;
    }
    //检查是否管理员账号，是则禁止修改
    if(data.at(0).data().toString()=="admin"){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("禁止修改管理员账号"), QMessageBox::tr("确定"));
        return;
    }
    //操作
    query.prepare("call UpdateUserName(?,?)");
    query.addBindValue(data.at(0).data().toString());
    query.addBindValue(ui->userIDInput->text());
    query.exec();
    if(query.lastError().isValid()){//名称重复
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("名称重复"), QMessageBox::tr("确定"));
        return;
    }
    mode->setQuery("select userid,favoritePlace from user",db);//更新
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("更新成功"), QMessageBox::tr("确定"));
}

void showUser::on_Output_clicked(const QModelIndex &index){//选中行
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//id
    ui->userIDInput->setText(data.at(0).data().toString());
}

