#include "floyddemonstrate.h"
#include "ui_floyddemonstrate.h"
#include <QTimer>
#include <QMessageBox.h>

int dist[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
QString NodeName[MAX_VERTEX_NUM];
int progressStep;
int nodeNum;

FloydDemonstrate::FloydDemonstrate(QWidget *parent): QDialog(parent), ui(new Ui::FloydDemonstrate){
    ui->setupUi(this);
    process = new QTimer();
    connect(process,&QTimer::timeout,this,&FloydDemonstrate::updatePath);
}

FloydDemonstrate::~FloydDemonstrate(){
    delete ui;
}

void FloydDemonstrate::execute(QString L[MAX_VERTEX_NUM],int d[MAX_VERTEX_NUM][MAX_VERTEX_NUM],int speed,int num){
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        NodeName[i] = L[i];
        for(int j=0;j<MAX_VERTEX_NUM;j++){
            dist[i][j] = d[i][j];
        }
    }
    nodeNum = num;
    //显示表格
    //设置行列
    ui->Output->clear();
    ui->Output->setRowCount(nodeNum);
    ui->Output->setColumnCount(nodeNum);
    //表头设置
    QStringList head;
    for(int i=0;i<nodeNum;i++){
        head<<NodeName[i];
    }
    ui->Output->setHorizontalHeaderLabels(head);
    ui->Output->setVerticalHeaderLabels(head);
    for(int i=0;i<nodeNum;i++){
        QTableWidgetItem *TableItem = new QTableWidgetItem("1000",1);
        ui->Output->setItem(0,i,TableItem);
    }
    //自动调整列宽行高
    ui->Output->resizeRowsToContents();
    ui->Output->resizeColumnsToContents();
    //设置内容
    for(int i=0;i<nodeNum;i++){
        for(int j=0;j<nodeNum;j++){
            QTableWidgetItem *TableItem;
            if(d[i][j]==INF)
                TableItem = new QTableWidgetItem("",1);
            else
                TableItem = new QTableWidgetItem(QString::number(d[i][j]),1);
            ui->Output->setItem(i,j,TableItem);
        }
    }
    //调用定时器
    progressStep=0;
    process->start(speed);
}

void FloydDemonstrate::updatePath(){
    if(progressStep<nodeNum){
        int j,k;
        for(j=0;j<nodeNum;j++){
            for(k=0;k<nodeNum;k++){
                if(dist[j][progressStep]+dist[progressStep][k]<dist[j][k]){//小于更新
                    dist[j][k] = dist[j][progressStep]+dist[progressStep][k];
                    //改变表格值
                    ui->Output->item(j,k)->setBackground(QBrush(QColor(220,20,60,100)));
                    ui->Output->item(j,k);
                    ui->Output->item(j,k)->setText(QString::number(dist[j][k]));
                }
            }
        }
        progressStep++;
    }else{
        process->stop();
        QMessageBox::information(this, QObject::tr("完成"), QObject::tr("演示完成"), QMessageBox::tr("确定"));
    }
}
