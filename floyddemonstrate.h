#ifndef FLOYDDEMONSTRATE_H
#define FLOYDDEMONSTRATE_H

#include <QDialog>

#define CUSTOM_VERTEX_NUM 30                    //可自己创建的节点数
#define ORIGINAL_VERTEX_NUM 34                  //原地图节点数
#define MAX_VERTEX_NUM ORIGINAL_VERTEX_NUM+CUSTOM_VERTEX_NUM     //最大节点数 = 原地图节点数+可自己创建的节点数
#define INF 0x3f3f3f3f

namespace Ui {
class FloydDemonstrate;
}

class FloydDemonstrate : public QDialog
{
    Q_OBJECT

public:
    explicit FloydDemonstrate(QWidget *parent = nullptr);
    ~FloydDemonstrate();
    QTimer *process;                                      //演示定时器
    void execute(QString Location[MAX_VERTEX_NUM],int d[MAX_VERTEX_NUM][MAX_VERTEX_NUM],int speed,int num);
    void updatePath();

private:
    Ui::FloydDemonstrate *ui;
};

#endif // FLOYDDEMONSTRATE_H
