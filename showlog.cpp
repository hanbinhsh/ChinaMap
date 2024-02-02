#include "showlog.h"
#include "qsqlquery.h"
#include "qsqlquerymodel.h"
#include "ui_showlog.h"
#include <QMessageBox.h>

static QSqlQueryModel *mode;
static QSqlDatabase db;//数据库

showLog::showLog(QSqlDatabase dbt,QWidget *parent): QDialog(parent), ui(new Ui::showLog){
    ui->setupUi(this);
    db = dbt;
    mode = new QSqlQueryModel(ui->Output);
    mode->setQuery("select * from log",db);
    mode->setHeaderData(0,Qt::Horizontal,tr("日志id"));
    mode->setHeaderData(1, Qt::Horizontal, tr("用户名"));
    mode->setHeaderData(2, Qt::Horizontal, tr("搜索起点"));
    mode->setHeaderData(3, Qt::Horizontal, tr("搜索终点"));
    mode->setHeaderData(4, Qt::Horizontal, tr("使用的算法"));
    ui->Output->setModel(mode);
    ui->Output->resizeRowsToContents();
    ui->Output->resizeColumnsToContents();
}

showLog::~showLog(){
    delete mode;
    delete ui;
}

void showLog::on_clearLog_clicked(){//删除日志
    QSqlQuery query(db);
    query.prepare("truncate log");
    query.exec();
    mode->setQuery("select * from log",db);//更新
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除完成"), QMessageBox::tr("确定"));
}


void showLog::on_deleteSelected_clicked(){//删除选中
    QSqlQuery query(db);
    QModelIndexList data;
    data = ui->Output->selectionModel()->selectedRows(0);//获取日志id列
    if(data.size()<=0){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择行"), QMessageBox::tr("确定"));
        return;
    }
    for(int i=0;i<data.size();i++){
        query.prepare("delete log from log where logid = ?");
        query.addBindValue(data.at(i).data(0).toInt());
        query.exec();
    }
    mode->setQuery("select * from log",db);//更新
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除完成"), QMessageBox::tr("确定"));
}

