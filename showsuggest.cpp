#include "showsuggest.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlquerymodel.h"
#include "ui_showsuggest.h"
#include <QMessageBox.h>

static QSqlQueryModel *mode;
static QSqlDatabase db;//数据库

showSuggest::showSuggest(QSqlDatabase dbt,QWidget *parent): QDialog(parent), ui(new Ui::showSuggest){
    ui->setupUi(this);
    db = dbt;
    mode = new QSqlQueryModel(ui->Output);
    mode->setQuery("select * from suggest",db);
    mode->setHeaderData(0,Qt::Horizontal,tr("地点"));
    mode->setHeaderData(1, Qt::Horizontal, tr("推荐信息"));
    ui->Output->setModel(mode);
    ui->Output->resizeColumnsToContents();
    ui->Output->resizeRowsToContents();
}

showSuggest::~showSuggest(){
    delete mode;
    delete ui;
}

void showSuggest::on_Output_clicked(const QModelIndex &index){//选中行
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//地点
    ui->place->setText(data.at(0).data().toString());
    data = ui->Output->selectionModel()->selectedRows(1);//信息
    ui->message->setText(data.at(0).data().toString());
}


void showSuggest::on_updateSelected_clicked(){//更新数据
    //选中检查
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//地点
    if(data.size()<=0){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择行"), QMessageBox::tr("确定"));
        return;
    }
    //输入检查
    if(ui->place->text().isEmpty()||ui->message->toPlainText().isEmpty()){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("输入不能为空"), QMessageBox::tr("确定"));
        return;
    }
    //更新
    QSqlQuery query(db);
    query.prepare("update suggest set place = ? where place = ?");
    query.addBindValue(ui->place->text());
    query.addBindValue(data.at(0).data().toString());
    query.exec();
    if(query.lastError().isValid()){//名称重复
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("名称重复"), QMessageBox::tr("确定"));
        return;
    }
    data = ui->Output->selectionModel()->selectedRows(1);//信息
    query.prepare("update suggest set message = ? where message = ?");
    query.addBindValue(ui->message->toPlainText());
    query.addBindValue(data.at(0).data().toString());
    query.exec();
    mode->setQuery("select * from suggest",db);
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("更新成功"), QMessageBox::tr("确定"));
}


void showSuggest::on_addSuggest_clicked(){//新建
    //输入检查
    if(ui->place->text().isEmpty()||ui->message->toPlainText().isEmpty()){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("输入不能为空"), QMessageBox::tr("确定"));
        return;
    }
    //更新
    QSqlQuery query(db);
    query.prepare("insert into suggest values(?,?)");
    query.addBindValue(ui->place->text());
    query.addBindValue(ui->message->toPlainText());
    query.exec();
    if(query.lastError().isValid()){//名称重复
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("名称重复"), QMessageBox::tr("确定"));
        return;
    }
    mode->setQuery("select * from suggest",db);
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("更新成功"), QMessageBox::tr("确定"));
}


void showSuggest::on_deleteSelected_clicked(){//删除选中
    //选中检查
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//地点
    if(data.size()<=0){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择行"), QMessageBox::tr("确定"));
        return;
    }
    //更新
    QSqlQuery query(db);
    query.prepare("delete suggest from suggest where place = ?");
    query.addBindValue(data.at(0).data().toString());
    query.exec();
    mode->setQuery("select * from suggest",db);
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除成功"), QMessageBox::tr("确定"));
}


void showSuggest::on_reset_clicked(){
    QSqlQuery query(db);
    query.prepare("call ResetSuggest");
    query.exec();
    mode->setQuery("select * from suggest",db);
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("重置成功"), QMessageBox::tr("确定"));
}

