#include "chinamap.h"
#include "qpainter.h"
#include "showlog.h"
#include "showsuggest.h"
#include "ui_chinamap.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QTimer>
#include <QFile>
#include "Graph.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QClipboard>
#include "QMessageBox.h"
//#include <QProgressDialog>
#include <QCryptographicHash>
#include "login.h"
#include "signin.h"
#include "changepassword.h"
#include "floyddemonstrate.h"
#include "showuser.h"
#include <QSettings>
#include "SetWindowCompositionAttribute.h"
#include <dwmapi.h>
#pragma comment ( lib,"user32.lib" )
#pragma comment ( lib,"dwmapi.lib" )

#define PAINTOFFSETX 187
#define PAINTOFFSETY 33

static MGraph G;
static int frontNode[MAX_VERTEX_NUM];                       //前面点数组（用于回溯路径）
static int toNode[MAX_VERTEX_NUM];                          //从输入点出发到其他点的最短长度数组
static int shortestPath[MAX_VERTEX_NUM];                    //最短路径走法（需要反向输出）
static int Start=0;                                         //输入的起始点
static int End=0;                                           //输入的结束点
static bool InputExceptuion;                                //输入异常检测,true时异常
static QRadioButton *startButton;                           //开始位置
static QRadioButton *endButton;                             //结束位置
static int frontPath;                                       //前面点
static int pathLength;                                      //路径长度
static int createdCustomButtonNum;                          //当前创建了的自定义节点数（表示当前创建了几个节点）
static bool enableCustomNodeSettings=false;                 //启用了自定义节点的设置
static int progressStep;                                    //步骤
static SqQueue Q_BFS;                                       //BFS辅助队列
static int dist[MAX_VERTEX_NUM][MAX_VERTEX_NUM];            //Floyd辅助数组
////自定义显示变量
static bool drawnPath=false;                                //开关最短路径显示
static bool ExecuseProgress=false;                          //算法过程显示
static bool ShowPathLength=false;                           //显示路线长度
static bool PathLengthAngle=false;                          //路线长度角度
static int PathLengthFontSize=10;                           //显示路线字体大小
static QColor PathLengthFontColor=QColor(65,105,225,200);   //显示路线字体颜色
static bool ShowNodeName=false;                             //显示节点名称
static int NodeNameFontSize=10;                             //显示名称字体大小
static QColor NodeNameFontColor=QColor(65,105,225,200);     //显示名称字体颜色
static QColor PathColor=QColor(47,79,79,80);                //路径颜色
static QColor ResultColor=QColor(255,0,0,250);              //结果颜色
static bool DarkMode = false;                               //黑暗主题
static int WindowOpacity=100;                               //透明度
static QColor PathColorProgress = QColor(255,165,0,150);    //算法演示的路径颜色
////数据库和excel
static int num_digits=2;                                    //保留小数位数
static int execuseTimes = 10;                               //保存算法执行过程时的执行次数
static QString excleFileName = "";                          //excel文件名
static QString userId = "";                                 //用户名(非空时表示登录)
static bool DBConnected=false;                              //数据库成功连接
static int logIDCount;                                      //记录logid

//MAX_VERTEX_NUM 最大节点数（可自定义节点数加地图原节点数）
//CUSTOM_VERTEX_NUM 自定义节点数

ChinaMap::ChinaMap(QWidget *parent): QMainWindow(parent), ui(new Ui::ChinaMap){//构造函数
    ui->setupUi(this);
    //初始化邻接矩阵
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        for(int j=0;j<MAX_VERTEX_NUM;j++){
            MapRelation[i][j] = INF;
        }
        MapRelation[i][i] = 0;
    }
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        for(int j=0;j<MAX_VERTEX_NUM;j++){
            if(MapRelation_O[i][j]!=0&&MapRelation_O[i][j]!=INF)
                MapRelation[i][j] = MapRelation_O[i][j];
        }
    }
    //关联信号
    for(int i=0;i<CUSTOM_VERTEX_NUM;i++)
        CustomizeButton[i]=NULL;
    for(int i=0;i<ORIGINAL_VERTEX_NUM;i++){//关联节点到getcheckbox槽
        QRadioButton *Node = findButton(i);
        connect(Node,SIGNAL(clicked(bool)),this,SLOT(getcheckbox()));
    }
    drawLines = new QTimer();
    connect(drawLines,&QTimer::timeout,this,&ChinaMap::drawNextLineSegment);
    drawLines_progress = new QTimer();
    connect(drawLines_progress,&QTimer::timeout,this,&ChinaMap::drawNextLine_progressSegment);
    connect(ui->YLocation,SIGNAL(valueChanged(int)),this,SLOT(SetYPosition()));
    connect(ui->XLocation,SIGNAL(valueChanged(int)),this,SLOT(SetXPosition()));
    connect(ui->SetNodeName,SIGNAL(clicked(bool)),this,SLOT(on_SetNodeName_clicked(bool isInit)));
    //切换算法清空绘图
    connect(ui->BFS,SIGNAL(clicked()),this,SLOT(switchAlgorithm()));
    connect(ui->DIJ,SIGNAL(clicked()),this,SLOT(switchAlgorithm()));
    connect(ui->Dijkstra_process,SIGNAL(clicked()),this,SLOT(switchAlgorithm()));
    connect(ui->BFS_process,SIGNAL(clicked()),this,SLOT(switchAlgorithm()));
    connect(ui->Floyd_process,SIGNAL(clicked()),this,SLOT(switchAlgorithm()));
    //云母效果
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setMicaEffect();//普通云母效果
    //自动读取主题被选中
    QSettings themeSettings("config.ini", QSettings::IniFormat);
    themeSettings.beginGroup("Theme");
    if(themeSettings.value("AutoLoadThemeSettings").toBool()){
        on_ReadThemeSettings_clicked();
        ui->AutoLoadThemeSettings->setChecked(true);
    }
    themeSettings.endGroup();
    //将节点加入选择框
    Update_CustomNodeComboBox();
    //数据库连接
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("ChinaMap");
    db.setUserName("root");
    db.setPassword("1024622242");
    bool ok = db.open();
    if (ok){//连接成功
        //QMessageBox::information(this, "infor", "success");
        DBConnected = true;
        //取得最大的logid
        QSqlQuery query(db);
        query.prepare("select logid from log order by logid desc limit 0,1");
        query.exec();
        if(!query.next()){//未查询到记录设置为0
            logIDCount = 1;
        }else{
            logIDCount = query.value("logid").toInt()+1;
        }
        //读入用户数据
        QSettings userSettings("config.ini", QSettings::IniFormat);
        userSettings.beginGroup("USER");
        bool autoLogin = userSettings.value("autoLogin").toBool();
        QString password  = userSettings.value("password").toString();
        if(autoLogin){//选择了自动登录
            QSqlQuery query(db);
            query.prepare("select userid from user where userid = ? and userpassword = ?");
            query.addBindValue(userSettings.value("userId").toString());query.addBindValue(password);
            query.exec();
            if(!query.next()){//未查询到记录
                QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("自动登录失败，请重新登录"), QMessageBox::tr("确定"));
                //关闭自动登录
                userSettings.setValue("autoLogin",false);
            }else{
                //登录成功
                userId = userSettings.value("userId").toString();
                ui->userId->setText(userId);
                updateSuggest();
                //enable按钮
                ui->ClearAccountData->setEnabled(true);
                ui->DeleteAccount->setEnabled(true);
                ui->LogOut->setEnabled(true);
                ui->ChangePassword->setEnabled(true);
                //检查管理员
                if(userId == "admin"){
                    ui->DeleteAccount->setEnabled(false);//禁用删除账号按钮
                }else{
                    ui->BottomTab->removeTab(6);//普通用户删除管理员Tab页
                }
            }
        }else{//未选择自动登录
            ui->BottomTab->removeTab(6);//删除管理员Tab页
        }
        userSettings.endGroup();
        qDebug()<<"DataBaseConnected"<<"CurrentUserId:"<<userId<<"CurrentLogID:"<<logIDCount;//调试信息
    }else{
        QMessageBox::information(this, "提示", "数据库连接失败");
        ui->BottomTab->removeTab(5);//删除账号Tab页
        ui->BottomTab->removeTab(6);//删除管理员Tab页
        qDebug()<<"error open database because"<<db.lastError().text();
    }
}

ChinaMap::~ChinaMap(){
    delete ui;
}
/*-------------------------------------------------------------------绘图部分----------------------------------------------------------------------*/
void ChinaMap::paintEvent(QPaintEvent *event){//绘图区域
    QPainter mapPainter(this);
    mapPainter.setRenderHint(QPainter::Antialiasing);//抗锯齿
    QPen mapPen;
    mapPen.setWidth(5);
    mapPen.setColor(PathColor);
    mapPainter.setPen(mapPen);
    for(int i=0;i<MAX_VERTEX_NUM;i++){//连线
        for(int j=0;j<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;j++){
            if(MapRelation[i][j]<INF&&MapRelation[i][j]!=0){
                QRadioButton *from = findButton(i);
                QRadioButton *to = findButton(j);
                if(from&&to){
                    if(from->isVisible()&&to->isVisible())
                        mapPainter.drawLine(PAINTOFFSETX+from->x(),PAINTOFFSETY+from->y(),PAINTOFFSETX+to->x(),PAINTOFFSETY+to->y());
                }else
                    qDebug()<<"paintEvent(QPaintEvent *event)_节点未找到";
            }
        }
    }
    if(ExecuseProgress){//执行过程
        QPainter progressPainter(this);
        progressPainter.setRenderHint(QPainter::Antialiasing);//抗锯齿
        QPen progressPen;
        progressPen.setWidth(5);
        progressPen.setColor(PathColorProgress);
        progressPainter.setPen(progressPen);
        if(ui->BFS_process->isChecked()||ui->Dijkstra_process->isChecked()){
            for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){
                //将visited点标色
                if(visited[i]){
                    progressPainter.drawEllipse(PAINTOFFSETX+findButton(i)->x()-7,PAINTOFFSETY+findButton(i)->y()-8,15,15);
                }
                //将toNode<INF的路径标色
                if(toNode[i]<INF&&frontNode[i]!=-1){
                    QRadioButton *from = findButton(frontNode[i]);
                    QRadioButton *to = findButton(i);
                    if(from&&to)
                        progressPainter.drawLine(PAINTOFFSETX+from->x(),PAINTOFFSETY+from->y(),PAINTOFFSETX+to->x(),PAINTOFFSETY+to->y());
                    else
                        qDebug()<<"paintEvent(QPaintEvent *event)_节点未找到";
                }
            }
        }
    }
    if(drawnPath){//画出最短路径（shortestPath控制）
        QPainter pathPainter(this);
        pathPainter.setRenderHint(QPainter::Antialiasing);//抗锯齿
        QPen pathPen;
        pathPen.setWidth(5);
        pathPen.setColor(ResultColor);
        pathPainter.setPen(pathPen);
        for(int i=pathLength-1;i>=frontPath;i--){
            QRadioButton *from = findButton(shortestPath[i]);
            QRadioButton *to = findButton(shortestPath[i-1]);
            if(from&&to)
                pathPainter.drawLine(PAINTOFFSETX+from->x(),PAINTOFFSETY+from->y(),PAINTOFFSETX+to->x(),PAINTOFFSETY+to->y());
            else
                qDebug()<<"paintEvent(QPaintEvent *event)_节点未找到";
        }
    }
    if(ShowPathLength){//画出路径长度
        QPainter lengthPainter(this);
        lengthPainter.setRenderHint(QPainter::Antialiasing);//抗锯齿
        QPen lengthPen;
        lengthPen.setWidth(5);
        lengthPen.setColor(PathLengthFontColor);
        lengthPainter.setFont(QFont(("黑体"),PathLengthFontSize));
        lengthPainter.setPen(lengthPen);
        for(int i=0;i<MAX_VERTEX_NUM;i++){
            for(int j=0;j<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;j++){
                if(MapRelation[i][j]<INF&&MapRelation[i][j]!=0){
                    QRadioButton *from = findButton(i);
                    QRadioButton *to = findButton(j);
                    if(from&&to){
                        lengthPainter.resetTransform();
                        double x=PAINTOFFSETX+from->x()+PAINTOFFSETX+to->x();
                        double y=PAINTOFFSETY+from->y()+PAINTOFFSETY+to->y();
                        if(PathLengthAngle){
                            double ox=from->x()-to->x();
                            double oy=from->y()-to->y();
                            double r;
                            if(!ox)
                                r=90;
                            else
                                r=atan(oy/ox)*180/3.14159;
                            lengthPainter.translate(x/2-PathLengthFontSize/2,y/2+PathLengthFontSize/2);
                            lengthPainter.rotate(r);
                            if(from->isVisible()&&to->isVisible())
                                lengthPainter.drawText(0,0,QString::number(MapRelation[i][j]));
                        }else{
                            lengthPainter.translate(x/2-PathLengthFontSize,y/2+PathLengthFontSize/2);
                            if(from->isVisible()&&to->isVisible())
                                lengthPainter.drawText(0,0,QString::number(MapRelation[i][j]));
                        }
                    }else
                        qDebug()<<"paintEvent(QPaintEvent *event)_节点未找到";
                }
            }
        }
    }
    if(ShowNodeName){//显示节点名称
        QPainter namePainter(this);
        namePainter.setRenderHint(QPainter::Antialiasing);//抗锯齿
        QPen namePen;
        namePen.setWidth(5);
        namePen.setColor(NodeNameFontColor);
        namePainter.setFont(QFont(("黑体"),NodeNameFontSize));
        namePainter.setPen(namePen);
        QList<QRadioButton*> nodes = ui->checkboxGroupBox->findChildren<QRadioButton*>();
        foreach(auto node, nodes){
            if(node->isVisible())
                namePainter.drawText(PAINTOFFSETX+6+node->x(),PAINTOFFSETY+4+node->y(),findNameByObjectName(node->objectName()));
        }
    }
}
/*-------------------------------------------------------------------绘图部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------计算显示部分----------------------------------------------------------------------*/
// TODO 向文本框和excel输出耗时
////正常计算
void ChinaMap::getcheckbox(){//获取选中输入框个数
    QObject *obj = QObject::sender();
    QRadioButton *chb = qobject_cast<QRadioButton *>(obj);//chb为发出按下信号的按钮
    if(startButton&&endButton&&chb->isChecked()){//如果开始和结束都已选择且要按下新点
        chb->toggle();
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("最多只能选择两个地点"), QMessageBox::tr("确定"));
    }else{
        if(chb->isChecked()){
            if(startButton){//如果开始已经选择
                ui->endInput->setText(findLocation(chb));//将开始文字框设置为选择的地点
                endButton = chb;
            }else{
                ui->startInput->setText(findLocation(chb));//将开始文字框设置为选择的地点
                startButton = chb;
            }
        }else{
            if(chb==endButton){//如果关掉的是结束位置
                ui->endInput->setText("");//清空结束输入框
                endButton = NULL;
            }else{
                ui->startInput->setText("");//清空开始输入框
                startButton = NULL;
            }
        }
    }
}

void ChinaMap::on_solve_clicked(){//计算
    if(!startButton||!endButton){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择起点或终点"), QMessageBox::tr("确定"));
    }else{
        //初始化图
        MatrixToGarcs(MapRelation,&G);
        //计算
        if(ui->DIJ->isChecked()){
            ui->timeConsuming->setText(QString::number(ShortestPath_DIJ(G,Start,End,toNode,frontNode)).first(5)+"ms");
            //数据库插入日志
            if(userId!=""){
                QSqlQuery query(db);
                query.prepare("INSERT INTO log (logid,userid,startplace,endplace,usedalgorithm) VALUES(?,?,?,?,'DIJ')");
                query.addBindValue(logIDCount);query.addBindValue(userId);query.addBindValue(Location[Start]);query.addBindValue(Location[End]);
                query.exec();
                logIDCount++;
            }
        }else if(ui->BFS->isChecked()){
            ui->timeConsuming->setText(QString::number(ShortestPath_BFS(G,Start,End,toNode,frontNode)).first(5)+"ms");
            //数据库插入日志
            if(userId!=""){
                QSqlQuery query(db);
                query.prepare("INSERT INTO log (logid,userid,startplace,endplace,usedalgorithm) VALUES(?,?,?,?,'BFS')");
                query.addBindValue(logIDCount);query.addBindValue(userId);query.addBindValue(Location[Start]);query.addBindValue(Location[End]);
                query.exec();
                logIDCount++;
            }
        }
        //刷新推荐
        updateSuggest();
        //显示最短路
        showShortestPath();
    }
}

void ChinaMap::showShortestPath(){//画出最短路径并显示长度
    //初始化
    ui->PathOutput->clear();
    //显示长度
    if(toNode[End]>=INF){//没有路径
        ui->Result->setText("无法到达");
        ui->TransitTimes->setText("无法到达");
        return;
    }
    ui->Result->setText(QString::number(toNode[End]));//路径长度
    //画出路径
    drawnPath=true;
    updateShortestPath();
    ui->TransitTimes->setText(QString::number(frontPath));//中转次数
    ui->PathOutput->append("中转次数:"+QString::number(frontPath));//打印中转次数到文本框
    ui->PathOutput->append("路径长度:"+QString::number(toNode[End]));//打印路径长度到文本框
    //打印路线
    if(ui->DIJ->isChecked())
        ui->PathOutput->append("最短路径路线:");
    if(ui->BFS->isChecked())
        ui->PathOutput->append("中转次数最少路径路线:");
    ui->PathOutput->insertPlainText(Location[shortestPath[frontPath]]);
    ui->PathOutput->insertPlainText("->"+Location[shortestPath[frontPath-1]]);
    update();
    if(!ui->PlayAnimationByStep->isChecked()){//未勾选逐步播放
        drawLines->start(ui->AnimationSpeed->value());
    }
}

void ChinaMap::updateShortestPath(){//创建最短路径数组（从frontNode中抽出路径）
    int j=0;
    for(int i=End;i!=Start;i=frontNode[i]){
        shortestPath[j]=i;
        j++;
    }
    shortestPath[j]=Start;
    j++;
    shortestPath[j]=-1;//标志遍历完成
    for(pathLength=0;shortestPath[pathLength]!=-1;pathLength++);
    frontPath=pathLength-1;
}

void ChinaMap::on_clear_clicked(){//清空开关
    drawnPath=false;
    ExecuseProgress = false;
    //重置输入输出
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//按钮重置
        QString NodeName = "checkbox_"+LocationSpell[i];
        QRadioButton *Node = ui->checkboxGroupBox->findChild<QRadioButton*>(NodeName);
        if(Node)
            Node->setChecked(false);
        else
            qDebug()<<"on_clear_clicked()_节点未找到";
    }
    ui->endInput->setText("");
    ui->startInput->setText("");
    //重置显示
    ui->Result->setText("0000");
    ui->TransitTimes->setText("0000");
    ui->timeConsuming->setText("00ms");
    startButton = NULL;
    endButton = NULL;
    update();
    drawLines->stop();//停止计时器并清空输出框
    ui->PathOutput->clear();
    frontPath = 0;//防止逐步播放导致闪退
    pathLength = 0;
}

void ChinaMap::switchAlgorithm(){//切换算法时重置绘图区域
    drawLines_progress->stop();//停止计时器
    drawLines->stop();
    drawnPath=false;
    ExecuseProgress = false;
    ui->Result->setText("0000");
    ui->TransitTimes->setText("0000");
    ui->timeConsuming->setText("00ms");
    update();
    ui->PathOutput->clear();
    frontPath = 0;//防止逐步播放导致闪退
    pathLength = 0;
}

void ChinaMap::on_AnimationSpeed_valueChanged(int value){//动画延迟显示
    ui->AnimationSpeedLabel->setText(QString::number(value)+"ms");
}

void ChinaMap::drawNextLineSegment(){//画下一条线
    if(frontPath>1){
        ui->PathOutput->insertPlainText("->"+Location[shortestPath[frontPath-2]]);
        update();
        frontPath--;
    }else{
        drawLines->stop();
    }
}

void ChinaMap::on_startInput_textChanged(const QString &arg1){//输入起点事件
    //查找按钮列表中是否有输入的按钮，有则将按钮设置为开始按钮并checked，没有则将此输入框标红，InputExceptuion设置为false，当计算按钮按下时，如果InputExceptuion就弹出错误窗口
    //在Location中查到到下标后在按钮组中查找
    QRadioButton *btn = findButtonByName(arg1);
    if(btn==NULL||btn->isEnabled()==false){
        InputExceptuion=true;
        ui->startInput->setStyleSheet("border-style: solid;border-top-color: transparent;border-right-color: transparent;border-left-color: transparent;border-bottom-color:rgba(255,0,0,255);border-bottom-width: 1.5px;margin-top:1px;background:rgba(255,0,0,50)");
    }else{
        btn->setChecked(true);
        startButton=btn;
        InputExceptuion=false;
        Start = findCityLocation(startButton);
        ui->startInput->setStyleSheet("");
    }
}

void ChinaMap::on_endInput_textChanged(const QString &arg1){//输入终点事件
    QRadioButton *btn = findButtonByName(arg1);
    if(btn==NULL||btn->isEnabled()==false){
        InputExceptuion=true;
        ui->endInput->setStyleSheet("border-style: solid;border-top-color: transparent;border-right-color: transparent;border-left-color: transparent;border-bottom-color:rgba(255,0,0,255);border-bottom-width: 1.5px;margin-top:1px;background:rgba(255,0,0,50)");
    }else{
        btn->setChecked(true);
        endButton=btn;
        InputExceptuion=false;
        End = findCityLocation(endButton);
        ui->endInput->setStyleSheet("");
    }
}
////执行过程
void ChinaMap::drawNextLine_progressSegment(){
    if(ui->Dijkstra_process->isChecked()){
        if(progressStep<ORIGINAL_VERTEX_NUM+createdCustomButtonNum){
            /*------在未标记节点中选择距离出发点最近的节点，标记，收录进最优路径集合------*/
            int minLength=INF;
            int minNode=Start;
            for(int j=0;j<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;j++){//在未访问的节点中选择距离出发点最近节点
                if(visited[j]==0&&toNode[j]<minLength){
                    minLength=toNode[j];
                    minNode=j;
                }
            }
            visited[minNode]=1;//标记已访问
            //显示
            update();
            if(ui->optimizeDijkstra->isChecked()){
                if(minNode==End) goto endDIJProgress;//优化，标记终点退出循环，舍弃找到到其他点的最短路功能
            }
            for(int w=FistAdjVex(G,minNode);w>=0;w=NextAdjVex(G,minNode,w)){//遍历该节点的邻接节点，更新最短距离
                if(!visited[w]){//该节点未访问，计算长度
                    int tempLength = toNode[minNode]+MapRelation[minNode][w];
                    if(tempLength<toNode[w]){
                        toNode[w]=tempLength;
                        frontNode[w]=minNode;
                    }
                }
            }
            progressStep++;
        }else{
            endDIJProgress:
            //显示路线
            if(toNode[End]>=INF){//没有路径
                ui->Result->setText("无法到达");
                ui->TransitTimes->setText("无法到达");
                return;
            }
            ui->Result->setText(QString::number(toNode[End]));//路径长度
            //画出路径
            drawnPath=true;
            updateShortestPath();
            ui->TransitTimes->setText(QString::number(frontPath));//中转次数
            update();
            drawLines->start(ui->AnimationSpeed_process->value());
            drawLines_progress->stop();
        }
    }else if(ui->BFS_process->isChecked()){
        if(!QueueEmpty(Q_BFS)){
            int u;
            DeQueue(&Q_BFS,&u);
            if(u==End){//已找到最短路
                goto endBFSProgress;
            }
            for(int w = FistAdjVex(G,u);w>=0;w=NextAdjVex(G,u,w))//访问下一层
                if(!visited[w]){
                    toNode[w] = toNode[u] + G.arcs[w][u].w;//记录长度
                    frontNode[w] = u;//记录前面点
                    visited[w]=1;
                    EnQueue(&Q_BFS,w);
                }
            update();
        }else{
            endBFSProgress:
            ClearQueue(&Q_BFS);
            //显示路线
            if(toNode[End]>=INF){//没有路径
                ui->Result->setText("无法到达");
                ui->TransitTimes->setText("无法到达");
                return;
            }
            ui->Result->setText(QString::number(toNode[End]));//路径长度
            //画出路径
            drawnPath=true;
            updateShortestPath();
            ui->TransitTimes->setText(QString::number(frontPath));//中转次数
            update();
            drawLines->start(ui->AnimationSpeed_process->value());
            drawLines_progress->stop();
        }
    }
}

void ChinaMap::on_Execute_process_clicked(){//执行过程
    ExecuseProgress = false;
    drawnPath=false;
    switchAlgorithm();
    //BFS和Dij需要选择起点和终点
    if(ui->Floyd_process->isChecked()){//flo算法
        //初始化图
        MatrixToGarcs(MapRelation,&G);
        FloydDemonstrate w(this);
        w.show();
        w.execute(Location,MapRelation,ui->AnimationSpeed_process->value(),ORIGINAL_VERTEX_NUM+createdCustomButtonNum);
        w.exec();
    }else{
        //起点终点检查
        if(!startButton||!endButton){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择起点或终点"), QMessageBox::tr("确定"));
            return;
        }
        //初始化图
        MatrixToGarcs(MapRelation,&G);
        //初始化节点信息
        for(int v=0;v<MAX_VERTEX_NUM;++v){
            visited[v]=0;
            toNode[v]=INF;
            frontNode[v]=-1;
        }
        ExecuseProgress = true;
        progressStep=0;
        if(ui->Dijkstra_process->isChecked()){//dij算法
            toNode[Start]=0;//出发点到自己的路径长度为0
            drawLines_progress->start(ui->AnimationSpeed_process->value());
        }else if(ui->BFS_process->isChecked()){//bfs算法
            InitQueue(&Q_BFS);
            EnQueue(&Q_BFS,Start);//初始节点入队
            toNode[Start] = 0;
            visited[Start] = 1;
            drawLines_progress->start(ui->AnimationSpeed_process->value());
        }
    }
}

void ChinaMap::on_AnimationSpeed_process_valueChanged(int value){//更新动画延迟显示数字
    ui->AnimationSpeedLabel_process->setText(QString::number(value)+"ms");
}
/*-------------------------------------------------------------------计算显示部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------寻找按钮部分----------------------------------------------------------------------*/
QString ChinaMap::findLocation(QRadioButton *btn){//按钮寻找名字
    for(int i=0;i<MAX_VERTEX_NUM;i++)
        if("checkbox_"+LocationSpell[i]==btn->objectName())
            return Location[i];
    return "";
}

int ChinaMap::findCityLocation(QRadioButton *btn){//按钮寻找下标
    if(btn==NULL)
        return 0;
    for(int i=0;i<MAX_VERTEX_NUM;i++)
        if("checkbox_"+LocationSpell[i]==btn->objectName())
            return i;
    return 0;
}

QRadioButton* ChinaMap::findButton(int index){//下标寻找按钮
    if(index<0) return NULL;
    return ui->checkboxGroupBox->findChild<QRadioButton*>("checkbox_"+LocationSpell[index]);
}

QRadioButton* ChinaMap::findButtonByName(QString name){//中文名寻找按钮
    int index;
    for(index=0;index<MAX_VERTEX_NUM;index++)
        if(Location[index]==name)
            break;
    if(index==MAX_VERTEX_NUM)//没找到
        return NULL;
    QRadioButton* btn = findButton(index);
    if(btn==NULL)
        return NULL;
    for(int i=0;i<MAX_VERTEX_NUM;i++)
        if("checkbox_"+LocationSpell[index]==btn->objectName())
            return btn;
    return NULL;
}

int ChinaMap::findButtonByName_Int(QString name){//中文名寻找按钮下标
    int index;
    for(index=0;index<MAX_VERTEX_NUM;index++)
        if(Location[index]==name)
            return index;
    return -1;
}

QString ChinaMap::findNameByObjectName(QString ObjName){//通过objectname找到节点中文名
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        if("checkbox_"+LocationSpell[i] == ObjName){
            return Location[i];
        }
    }
    return "";
}
/*-------------------------------------------------------------------寻找按钮部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------动画播放设置部分----------------------------------------------------------------------*/
////播放
void ChinaMap::on_PlayAnimationByStep_stateChanged(int arg1){//逐步播放按钮改变状态
    ui->AnimationSpeed->setEnabled(!arg1);
    ui->NextStep->setEnabled(arg1);
    ui->PreStep->setEnabled(arg1);
    ui->AnimationSpeedLabel->setEnabled(!arg1);
    ui->AnimationSpeed_Tip->setEnabled(!arg1);
}

void ChinaMap::on_NextStep_clicked(){//下一步
    if(frontPath>1){
        ui->PathOutput->insertPlainText("->"+Location[shortestPath[frontPath-2]]);
        update();
        frontPath-=1;
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("已经是最后一步"), QMessageBox::tr("确定"));
    }
}

void ChinaMap::on_PreStep_clicked(){//上一步
    if(frontPath<pathLength){
        //删除输入框一步内容
        ui->PathOutput->moveCursor(QTextCursor::PreviousWord,QTextCursor::KeepAnchor);
        while(!ui->PathOutput->textCursor().selectedText().startsWith("->")){
            ui->PathOutput->moveCursor(QTextCursor::PreviousWord,QTextCursor::KeepAnchor);
        }
        ui->PathOutput->textCursor().removeSelectedText();
        update();
        frontPath+=1;
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("已经是最初一步"), QMessageBox::tr("确定"));
    }
}
/*-------------------------------------------------------------------动画播放设置部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------自定义节点部分----------------------------------------------------------------------*/
void ChinaMap::on_createNode_clicked(){//创建节点
    if(createdCustomButtonNum>=CUSTOM_VERTEX_NUM){//自定义节点到达最大数量警告
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("不能创建更多的节点"), QMessageBox::tr("确定"));
        return;
    }
    CustomizeButton[createdCustomButtonNum] = new QRadioButton("",ui->checkboxGroupBox);                 //将自定义节点加入checkboxGroupBox
    CustomizeButton[createdCustomButtonNum]->setAutoExclusive(false);
    connect(CustomizeButton[createdCustomButtonNum],SIGNAL(clicked(bool)),this,SLOT(getcheckbox()));
    if(on_SetNodeName_clicked(true)){//未重名
        SetPosition();
        CustomizeButton[createdCustomButtonNum]->setVisible(true);
        ui->CustomNodeSelect->addItem(Location[ORIGINAL_VERTEX_NUM+createdCustomButtonNum]);
        ui->CustomNodeSelect->setCurrentIndex(createdCustomButtonNum);
        createdCustomButtonNum++;
        Update_CustomNodeComboBox();
        ui->NodeName->setText("");//清空输入框
        if(enableCustomNodeSettings==false){//启用相关按钮
            enableCustomNodeSettings=true;
            EnableCustomNodeSettings(true);
        }
    }else{//重名
        delete CustomizeButton[createdCustomButtonNum];
        CustomizeButton[createdCustomButtonNum]=NULL;
    }
}

void ChinaMap::on_SetPath_clicked(){//设置到达地点长度（自定义节点）
    int from = findButtonByName_Int(ui->CustomNodeSelect->currentText());//节点选择下拉框下标
    int to = findButtonByName_Int(ui->SetLocationComboBox->currentText());//到达地点下拉框下标
    if(from<0||to<0) return;
    if(from==to){//如果输入的节点是当前节点
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("无法设置自身路径"), QMessageBox::tr("确定"));
        return;
    }
    setPathLength(from,to,ui->NodeLength->value());
}

void ChinaMap::SetXPosition(){//设置自定义点X位置
    int CurrentNodeIndex = ui->CustomNodeSelect->currentIndex();
    if(CurrentNodeIndex==-1) CurrentNodeIndex=0;
    if(CustomizeButton[CurrentNodeIndex]){
        CustomizeButton[CurrentNodeIndex]->setGeometry(ui->XLocation->value(),CustomizeButton[CurrentNodeIndex]->y(),25,25);
        update();
    }
}

void ChinaMap::SetYPosition(){//设置自定义点Y位置
    int CurrentNodeIndex = ui->CustomNodeSelect->currentIndex();
    if(CurrentNodeIndex==-1) CurrentNodeIndex=0;
    if(CustomizeButton[CurrentNodeIndex]){
        CustomizeButton[CurrentNodeIndex]->setGeometry(CustomizeButton[CurrentNodeIndex]->x(),ui->YLocation->value(),25,25);
        update();
    }
}

void ChinaMap::SetPosition(){//设置自定义点位置
    int CurrentNodeIndex = ui->CustomNodeSelect->currentIndex();
    if(CurrentNodeIndex==-1) CurrentNodeIndex=0;
    if(CustomizeButton[CurrentNodeIndex]){
        CustomizeButton[CurrentNodeIndex]->setGeometry(ui->XLocation->value(),ui->YLocation->value(),25,25);
        update();
    }
}

void ChinaMap::on_CustomNodeSelect_currentIndexChanged(int index){//选择自定义节点框更新
    if(index!=-1){
        ui->XLocation->setValue(CustomizeButton[index]->x());
        ui->YLocation->setValue(CustomizeButton[index]->y());
        if(ui->BottomTab->currentIndex()==2){//如果Tab页为自定义节点页
            for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++)//恢复样式表
                if(findButton(i)&&findButton(i)->styleSheet()=="QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:red;color: #a9b7c6;background-color: red;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: red;color: #a9b7c6;background-color: #F8F8FF;}}")
                    findButton(i)->setStyleSheet("");
            //标红选中的节点
            QRadioButton *Node = CustomizeButton[index];
            if(Node)
                Node->setStyleSheet("QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:red;color: #a9b7c6;background-color: red;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: red;color: #a9b7c6;background-color: #F8F8FF;}}");
        }
    }
}

void ChinaMap::Update_CustomNodeComboBox(){//更新两个到达地点下拉框和自定义地图的节点选择下拉框
    ui->SetLocationComboBox->clear();
    ui->SetLocationComboBox_Map->clear();
    ui->MapNodeSelect->clear();
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//将所有节点加入[到达节点]下拉菜单，包括**已经创建的**自定义节点，***在创建新节点,增加删除节点的时候更新***
        QString s=Location[i];
        if(findButton(i)->isEnabled()){
            ui->SetLocationComboBox->addItem(s);
            ui->SetLocationComboBox_Map->addItem(s);
            ui->MapNodeSelect->addItem(s);
        }
    }
}

void ChinaMap::on_DeleteCustomNode_clicked(){//删除当前选中的节点
    on_clear_clicked();
    int CurrentNodeIndex = findCityLocation(findButtonByName(ui->CustomNodeSelect->currentText()))-ORIGINAL_VERTEX_NUM;//找到对应名称，而不是下标寻找
    if(CurrentNodeIndex<0) CurrentNodeIndex=0;
    if(createdCustomButtonNum<1){//createdCustomButtonNum=0则报错
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("已经删除了所有节点"), QMessageBox::tr("确定"));
        return;
    }else{
        delete CustomizeButton[CurrentNodeIndex];//释放空间
        CustomizeButton[CurrentNodeIndex]=NULL;//置为空指针
        //数组删除元素的移位
        for(int i=ORIGINAL_VERTEX_NUM+CurrentNodeIndex;i<MAX_VERTEX_NUM-1;i++){
            CustomizeButton[i-ORIGINAL_VERTEX_NUM]=CustomizeButton[i+1-ORIGINAL_VERTEX_NUM];
            Location[i]=Location[i+1];
            LocationSpell[i]=LocationSpell[i+1];
            for(int j=0;j<i;j++){
                MapRelation[i][j]=MapRelation[i+1][j];
                MapRelation[j][i]=MapRelation[j][i+1];
            }
        }
        //将节点信息改为默认值
        for(int i=ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i<MAX_VERTEX_NUM;i++){
            MapRelation[i][ORIGINAL_VERTEX_NUM+createdCustomButtonNum]=INF;
            MapRelation[ORIGINAL_VERTEX_NUM+createdCustomButtonNum][i]=INF;
        }
        MapRelation[ORIGINAL_VERTEX_NUM+createdCustomButtonNum][ORIGINAL_VERTEX_NUM+createdCustomButtonNum]=0;
        createdCustomButtonNum--;
        ui->CustomNodeSelect->removeItem(CurrentNodeIndex);
        Update_CustomNodeComboBox();
        update();
        if(createdCustomButtonNum==0){//禁用相关按钮
            enableCustomNodeSettings=false;
            EnableCustomNodeSettings(false);
        }
    }
}

bool ChinaMap::on_SetNodeName_clicked(bool isInit){//设置节点名称
    int CurrentNodeIndex = ui->CustomNodeSelect->currentIndex();
    if(isInit)
        CurrentNodeIndex=createdCustomButtonNum;
    if(CurrentNodeIndex==-1)
        CurrentNodeIndex=0;
    QString temp = ui->NodeName->text();
    if(temp.isEmpty()){//未输入节点名称
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("请输入节点名称"), QMessageBox::tr("确定"));
        return false;
    }
    for(int i=0;i<MAX_VERTEX_NUM;i++){//重名
        if(temp==Location[i]||temp==LocationSpell[i]){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("节点名称重复"), QMessageBox::tr("确定"));
            return false;
        }
    }
    if(CustomizeButton[CurrentNodeIndex]==NULL){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择节点"), QMessageBox::tr("确定"));
        return false;
    }
    CustomizeButton[CurrentNodeIndex]->setObjectName("checkbox_"+temp);//设置节点名
    CustomizeButton[CurrentNodeIndex]->setToolTip(temp);
    Location[ORIGINAL_VERTEX_NUM+CurrentNodeIndex] = temp;
    LocationSpell[ORIGINAL_VERTEX_NUM+CurrentNodeIndex] = temp;
    ui->CustomNodeSelect->setItemText(CurrentNodeIndex,temp);
    Update_CustomNodeComboBox();
    update();
    return true;
}

void ChinaMap::on_SetLocationComboBox_currentIndexChanged(int index){//选择地点，更新自定义节点到地点路径长度下拉框
    if(ui->BottomTab->currentIndex()==2){
        for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++)//恢复样式表
            if(findButton(i)&&findButton(i)->styleSheet()=="QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:blue;color: #a9b7c6;background-color: blue;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: blue;color: #a9b7c6;background-color: #F8F8FF;}}")
                findButton(i)->setStyleSheet("");
        QRadioButton *Node = findButton(index);
        if(Node)//标蓝选中的节点
            Node->setStyleSheet("QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:blue;color: #a9b7c6;background-color: blue;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: blue;color: #a9b7c6;background-color: #F8F8FF;}}");
    }
}

void ChinaMap::on_BottomTab_currentChanged(int index){//改变了Tab页的编号（如果自定义节点或自定义地图界面，则将按钮样式表设置为空）（如果为算法分析页则禁用输入部分的相关按钮）
    if(index==2){//自定义节点或自定义地图界面
        on_SetLocationComboBox_currentIndexChanged(ui->SetLocationComboBox->currentIndex());
        on_CustomNodeSelect_currentIndexChanged(ui->CustomNodeSelect->currentIndex());
    }else if(index==3){
        on_SetLocationComboBox_Map_currentIndexChanged(ui->SetLocationComboBox_Map->currentIndex());
        on_MapNodeSelect_currentIndexChanged(ui->MapNodeSelect->currentIndex());
    }else{//不在自定义界面
        for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++)//恢复样式表
            if(findButton(i))
                findButton(i)->setStyleSheet("");
    }
    if(index==4){//算法分析页
        drawLines_progress->stop();
        drawLines->stop();
        drawnPath = false;
        ExecuseProgress = false;
        ui->BFS->setEnabled(false);
        ui->DIJ->setEnabled(false);
        ui->solve->setEnabled(false);
        ui->timeConsuming->setEnabled(false);
        ui->timeConsuming_Tip->setEnabled(false);
        switchAlgorithm();
    }else{
        drawLines_progress->stop();
        drawLines->stop();
        drawnPath = false;
        ExecuseProgress = false;
        ui->BFS->setEnabled(true);
        ui->DIJ->setEnabled(true);
        ui->solve->setEnabled(true);
        ui->timeConsuming->setEnabled(true);
        ui->timeConsuming_Tip->setEnabled(true);
        switchAlgorithm();
    }
}

void ChinaMap::EnableCustomNodeSettings(bool enable){//启用关闭自定义节点设置
    ui->CustomNodeSelect->setEnabled(enable);
    ui->CustomNodeSelect_Tip->setEnabled(enable);
    ui->SetLocationComboBox->setEnabled(enable);
    ui->SetLocationComboBox_Tip->setEnabled(enable);
    ui->XLocation->setEnabled(enable);
    ui->XLocation_Tip->setEnabled(enable);
    ui->YLocation->setEnabled(enable);
    ui->YLocation_Tip->setEnabled(enable);
    ui->NodeLength->setEnabled(enable);
    ui->NodeLength_Tip->setEnabled(enable);
    ui->SetPath->setEnabled(enable);
    ui->DeleteCustomNode->setEnabled(enable);
    ui->SetNodeName->setEnabled(enable);
}

void ChinaMap::SetPosition(int x,int y,int nodeIndex){//设置点位置（自定义地图）
    if(nodeIndex>=0){
        QRadioButton *btn = findButton(nodeIndex);
        if(btn==NULL) return;
        btn->setGeometry(x,y,25,25);
        update();
    }
}

void ChinaMap::on_SetXPosition_valueChanged(int arg1){//修改地图点的X改变
    int nodeIndex = ui->MapNodeSelect->currentIndex();
    SetPosition(arg1,findButton(nodeIndex)->y(),nodeIndex);
}

void ChinaMap::on_SetYPosition_valueChanged(int arg1){//修改地图点的Y改变
    int nodeIndex = ui->MapNodeSelect->currentIndex();
    SetPosition(findButton(nodeIndex)->x(),arg1,nodeIndex);
}

void ChinaMap::on_MapNodeSelect_currentIndexChanged(int index){//更新了节点选择下拉输入框
    if(index == -1) return;
    ui->SetXPosition->setValue(findButton(index)->x());
    ui->SetYPosition->setValue(findButton(index)->y());
    if(ui->BottomTab->currentIndex()==3){//如果Tab页为自定义地图页
        for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++)//恢复样式表
            if(findButton(i)&&findButton(i)->styleSheet()=="QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:red;color: #a9b7c6;background-color: red;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: red;color: #a9b7c6;background-color: #F8F8FF;}}")
                findButton(i)->setStyleSheet("");
        QRadioButton *Node = findButton(index);
        if(Node)//标红选中的节点
            Node->setStyleSheet("QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:red;color: #a9b7c6;background-color: red;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: red;color: #a9b7c6;background-color: #F8F8FF;}}");
    }
}

void ChinaMap::on_SetLocationComboBox_Map_currentIndexChanged(int index){//更新了地点选择下拉输入框
    if(ui->BottomTab->currentIndex()==3){
        for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++)//恢复样式表
            if(findButton(i)&&findButton(i)->styleSheet()=="QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:blue;color: #a9b7c6;background-color: blue;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: blue;color: #a9b7c6;background-color: #F8F8FF;}}")
                findButton(i)->setStyleSheet("");
        QRadioButton *Node = findButton(index);
        // BUG 切换选择节点到其他节点时北京不会标蓝，尝试初始化为成都为蓝色
        if(Node)//标蓝选中的节点
            Node->setStyleSheet("QRadioButton::indicator:checked {height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color:blue;color: #a9b7c6;background-color: blue;}QRadioButton::indicator:!checked{height: 10px;width: 10px;border-style:solid;border-radius:5px;border-width: 1px;border-color: blue;color: #a9b7c6;background-color: #F8F8FF;}}");
    }
}

void ChinaMap::on_SetPath_Map_clicked(){//设置路径长度（自定义地图）
    int from = ui->MapNodeSelect->currentIndex();
    int to = ui->SetLocationComboBox_Map->currentIndex();
    if(from==to){//如果输入的节点是当前节点
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("无法设置自身路径"), QMessageBox::tr("确定"));
        return;
    }
    setPathLength(from,to,ui->NodeLength_Map->value());
}

void ChinaMap::setPathLength(int nodeFrom,int nodeTo,int length){//设置两点间的路径长度
    if(length==0){//设置为0就为无穷大
        MapRelation[nodeFrom][nodeTo]=INF;
        MapRelation[nodeTo][nodeFrom]=INF;
    }else{
        MapRelation[nodeFrom][nodeTo]=length;
        MapRelation[nodeTo][nodeFrom]=length;
    }
    on_clear_clicked();
}

void ChinaMap::on_SetNodeName_Map_clicked(){//重命名（自定义地图）
    int CurrentNodeIndex = ui->MapNodeSelect->currentIndex();
    QString name = ui->NodeName_Map->text();
    if(name.isEmpty()){//未输入节点名称
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("请输入节点名称"), QMessageBox::tr("确定"));
        return;
    }
    NodeRename(CurrentNodeIndex,name);
}

bool ChinaMap::NodeRename(int node,QString name){//设置节点名称
    //自定义节点可调用此函数（bug）
    for(int i=0;i<MAX_VERTEX_NUM;i++){//重名
        if(name==Location[i]||name==LocationSpell[i]){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("节点名称重复"), QMessageBox::tr("确定"));
            return false;
        }
    }
    findButton(node)->setToolTip(name);
    findButton(node)->setObjectName("checkbox_"+name);
    Location[node] = name;
    LocationSpell[node] = name;
    if(node>=ORIGINAL_VERTEX_NUM)
        ui->CustomNodeSelect->setItemText(node-ORIGINAL_VERTEX_NUM,name);
    Update_CustomNodeComboBox();
    update();
    return true;
}

void ChinaMap::on_enableOriNode_stateChanged(int arg1){//关闭系统点显示
    for(int i=0;i<ORIGINAL_VERTEX_NUM;i++){
        findButton(i)->setVisible(arg1);
        findButton(i)->setEnabled(arg1);
    }
    //禁用自定义地图相关设置
    ui->ORINodeEdit->setEnabled(arg1);
    //禁用的点不加入下拉框和输入检测
    Update_CustomNodeComboBox();
    on_clear_clicked();
    update();
}

void ChinaMap::on_changeMapPhoto_clicked(){//更换地图
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="选择图片"; //对话框标题
    QString filter="图片(*.png *.jpg *.bmp)"; //文件过滤器
    QString fileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".png")||fileName.endsWith(".jpg")||fileName.endsWith(".bmp")){
        ui->map->setStyleSheet("border-image: url(" + fileName + ")");
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
}
/*-------------------------------------------------------------------自定义节点部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------文件读取部分----------------------------------------------------------------------*/
void ChinaMap::on_ReadCustomNodeFromFile_clicked(){//读取自定义节点文件
    QDialog loading(this);
    loading.setWindowTitle("加载中");
    loading.setMaximumHeight(1);
    loading.open();
    QString curPath=QDir::currentPath();//获取系统当前目录
    QString dlgTitle="选择自定义节点信息文件"; //对话框标题
    QString filter="Microsoft Excel工作表(*.xlsx);;Microsoft Excel 97-2003 工作表(*.xls)"; //文件过滤器
    QString fileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".xlsx")||fileName.endsWith(".xls")){
        setCustomNodeFromFile(fileName);
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
    loading.close();
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("读取自定义节点完成"), QMessageBox::tr("确定"));
}

void ChinaMap::setCustomNodeFromFile(QString fileName){//读取自定义节点文件
    //打开excel并进行初始化
    excel = new QAxObject("Excel.Application");
    works = excel->querySubObject("WorkBooks");
    works->dynamicCall("Open(const QString&)",QDir::toNativeSeparators(fileName));//QDir将路径中的"/"转换为"\"
    workbook = excel->querySubObject("ActiveWorkBook");
    sheet = workbook->querySubObject("Sheets(int)",1);
    //读取数据部分
    QAxObject *nodeNameCell = sheet->querySubObject("Range(QVariant,QVariant)","B1");
    QAxObject *nodeXPositionCell = sheet->querySubObject("Range(QVariant,QVariant)","C4");
    QAxObject *nodeYPositionCell = sheet->querySubObject("Range(QVariant,QVariant)","C5");
    QString nodeName = nodeNameCell->dynamicCall("Value2()").toString();
    int nodeXPosition = nodeXPositionCell->dynamicCall("Value2()").toInt();
    int nodeYPosition = nodeYPositionCell->dynamicCall("Value2()").toInt();
    int offset = 6;
    int nameStart = 1,xStart = 4,yStart = 5,count = 1;
    //先加入节点再设置路径
    //加入节点
    while(nodeName!=""){
        ui->NodeName->setText(nodeName);//设置名称
        on_createNode_clicked();//新建节点
        ui->XLocation->setValue(nodeXPosition);//设置节点位置   处理空和超范围的情况?(在不乱改自定义节点文件的情况下，处理这两种情况是没有意义的)
        ui->YLocation->setValue(nodeYPosition);
        nodeNameCell = sheet->querySubObject("Range(QVariant,QVariant)","B"+QString::number(nameStart+count*offset));
        nodeXPositionCell = sheet->querySubObject("Range(QVariant,QVariant)","C"+QString::number(xStart+count*offset));
        nodeYPositionCell = sheet->querySubObject("Range(QVariant,QVariant)","C"+QString::number(yStart+count*offset));
        nodeName = nodeNameCell->dynamicCall("Value2()").toString();
        nodeXPosition = nodeXPositionCell->dynamicCall("Value2()").toInt();
        nodeYPosition = nodeYPositionCell->dynamicCall("Value2()").toInt();
        count++;
    }
    //设置路径
    nodeNameCell = sheet->querySubObject("Range(QVariant,QVariant)","B1");//复位
    nodeName = nodeNameCell->dynamicCall("Value2()").toString();
    int pathCurrentRow = 2,pathLengthCurrentRow = 3;
    count = 1;
    while(nodeName!=""){
        char pathCurrentColumn_0='C';//路径低位复位
        char pathCurrentColumn_1='\0';//路径列高位复位
        int flag=0;
        char pathCurrentColumn[3] = {pathCurrentColumn_0,pathCurrentColumn_1,0};
        QAxObject *nodePathCell = sheet->querySubObject("Range(QVariant,QVariant)",pathCurrentColumn+QString::number(pathCurrentRow));
        QAxObject *pathLengthCell = sheet->querySubObject("Range(QVariant,QVariant)",pathCurrentColumn+QString::number(pathLengthCurrentRow));
        QString nodePath = nodePathCell->dynamicCall("Value2()").toString();
        int pathLength = pathLengthCell->dynamicCall("Value2()").toInt();
        while(nodePath!=""){//设置路径
            ui->NodeLength->setValue(pathLength);//长度输入
            ui->CustomNodeSelect->setCurrentText(nodeName);//节点选择
            ui->SetLocationComboBox->setCurrentText(nodePath);//到达地点
            on_SetPath_clicked();//设置长度
            if(pathCurrentColumn_0-'A'>=25){//超过了26
                pathCurrentColumn_1='A';
                pathCurrentColumn_0='A';
                flag=1;
            }else if(flag==0){
                pathCurrentColumn_0++;
            }else{
                pathCurrentColumn_1++;
                if(pathCurrentColumn_1-'A'>=26){
                    pathCurrentColumn_1='A';
                    pathCurrentColumn_0++;
                }
            }
            pathCurrentColumn[0] = pathCurrentColumn_0;
            pathCurrentColumn[1] = pathCurrentColumn_1;
            nodePathCell = sheet->querySubObject("Range(QVariant,QVariant)",pathCurrentColumn+QString::number(pathCurrentRow));
            pathLengthCell = sheet->querySubObject("Range(QVariant,QVariant)",pathCurrentColumn+QString::number(pathLengthCurrentRow));
            nodePath = nodePathCell->dynamicCall("Value2()").toString();
            pathLength = pathLengthCell->dynamicCall("Value2()").toInt();
        }
        nodeNameCell = sheet->querySubObject("Range(QVariant,QVariant)","B"+QString::number(nameStart+count*offset));
        nodeName = nodeNameCell->dynamicCall("Value2()").toString();
        pathCurrentRow+=offset,pathLengthCurrentRow+=offset;
        count++;
    }
    //关闭excel
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    delete excel;
}
/*-------------------------------------------------------------------文件读取部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------文件保存部分----------------------------------------------------------------------*/
void ChinaMap::on_copyOutput_clicked(){//输出到剪切板
    QClipboard *clip = QApplication::clipboard();
    clip->setText(ui->PathOutput->toPlainText());
}
////自定义节点的保存
void ChinaMap::on_SaveCustomNodeToFile_clicked(){//保存自定义节点文件
    //loading
    QDialog loading(this);
    loading.setWindowTitle("加载中");
    loading.setMaximumHeight(1);
    loading.open();
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存自定义节点信息文件"; //对话框标题
    QString filter="Microsoft Excel工作表(*.xlsx);;Microsoft Excel 97-2003 工作表(*.xls)"; //文件过滤器
    QString fileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".xlsx")||fileName.endsWith(".xls")){
        excleFileName = fileName;
        saveCustomNode();
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
    loading.close();
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("保存自定义节点完成"), QMessageBox::tr("确定"));
}

void ChinaMap::saveCustomNode(){//保存自定义节点文件
    //打开excel并进行初始化
    initSaveExcel();
    //保存数据部分
    int offset = 6;
    int nameStart = 1,xStart = 4,yStart = 5;
    int pathCurrentRow = 2,pathLengthCurrentRow = 3;
    for(int i=0;i<createdCustomButtonNum;i++){
        ////初始化提升信息
        //设置节点名称
        cellCentreAndText("A"+QString::number(nameStart+i*offset),"节点名称");
        //设置路径信息
        cellCentreMergeAndText("=(A" + QString::number(pathCurrentRow+i*offset) +": A" + QString::number(pathLengthCurrentRow+i*offset) + ")","路径信息");
        cellCentreAndText("B"+QString::number(pathCurrentRow+i*offset),"到达地点");
        cellCentreAndText("B"+QString::number(pathLengthCurrentRow+i*offset),"路径长度");
        //设置节点位置
        cellCentreMergeAndText("=(A" + QString::number(xStart+i*offset) +": A" + QString::number(yStart+i*offset) + ")","位置");
        cellCentreAndText("B"+QString::number(xStart+i*offset),"x位置");
        cellCentreAndText("B"+QString::number(yStart+i*offset),"y位置");
        ////写入节点信息
        //设置节点名称
        cellCentreAndText("B"+QString::number(nameStart+i*offset),Location[ORIGINAL_VERTEX_NUM+i]);
        //设置路径信息
        char pathCurrentColumn_0='B';//路径低位复位
        char pathCurrentColumn_1='\0';//路径列高位复位
        int flag=0;
        char pathCurrentColumn[3] = {pathCurrentColumn_0,pathCurrentColumn_1,0};
        for(int j=0;j<MAX_VERTEX_NUM;j++){//遍历图矩阵中当前节点的行
            if(MapRelation[ORIGINAL_VERTEX_NUM+i][j]!=INF&&MapRelation[ORIGINAL_VERTEX_NUM+i][j]!=0){//能够到达则写入
                if(pathCurrentColumn_0-'A'>=25){//超过了26
                    pathCurrentColumn_1='A';
                    pathCurrentColumn_0='A';
                    flag=1;
                }else if(flag==0){
                    pathCurrentColumn_0++;
                }else{
                    pathCurrentColumn_1++;
                    if(pathCurrentColumn_1-'A'>=26){
                        pathCurrentColumn_1='A';
                        pathCurrentColumn_0++;
                    }
                }
                pathCurrentColumn[0] = pathCurrentColumn_0;
                pathCurrentColumn[1] = pathCurrentColumn_1;
                cellCentreAndText(pathCurrentColumn+QString::number(pathCurrentRow+i*offset),Location[j]);
                cellCentreAndText(pathCurrentColumn+QString::number(pathLengthCurrentRow+i*offset),QString::number(MapRelation[ORIGINAL_VERTEX_NUM+i][j]));
            }
        }
        //设置节点位置
        cellCentreAndText("C"+QString::number(xStart+i*offset),QString::number(CustomizeButton[i]->x()));
        cellCentreAndText("C"+QString::number(yStart+i*offset),QString::number(CustomizeButton[i]->y()));
    }
    //操作结束
    saveAndCloseExcel();
}
////图分析文件的保存
void ChinaMap::on_crateGraphAnalyseFile_clicked(){
    // TODO loading使用QProgressDialog?
    //loading
    QDialog loading(this);
    loading.setWindowTitle("加载中");
    loading.setMaximumHeight(1);
    loading.open();
    //执行算法
    MatrixToGarcs(MapRelation,&G);
    ShortestPath_FLOYD(G,dist);
    //连通性检查
    int connectivityCheck=0;//不连通点计数
    for(int j=0;j<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;j++){
        if(dist[0][j]==INF){
            connectivityCheck++;
        }
    }
    if(connectivityCheck){
        QString s = "有"+QString::number(connectivityCheck)+"个节点不连通，图不连通可能影响分析结果，确定要继续吗？";
        int choice = QMessageBox::warning(this, QObject::tr("警告"), s, QMessageBox::tr("确定"), QMessageBox::tr("取消"));
        if(choice){//选择了取消
            return;
        }
    }
    //保存
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存地图分析文件"; //对话框标题
    QString filter="Microsoft Excel工作表(*.xlsx);;Microsoft Excel 97-2003 工作表(*.xls)"; //文件过滤器
    QString fileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".xlsx")||fileName.endsWith(".xls")){
        excleFileName = fileName;
        saveGraphAnalyseFile();
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
    //完成
    loading.close();
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("生成地图分析文件完成"), QMessageBox::tr("确定"));
}

void ChinaMap::saveGraphAnalyseFile(){//保存地图分析文件
    //打开excel并进行初始化
    initSaveExcel();
    //保存数据部分
    ////写入列
    int currentColumn = MAX_VERTEX_NUM-CUSTOM_VERTEX_NUM+createdCustomButtonNum+1;
    int rowStart = 2;//行开始（除去第一行名称）
    for(int i=0;i<currentColumn;i++){
        cellCentreAndText("A"+QString::number(rowStart+i),Location[i]);
    }
    ////写入行
    char pathCurrentColumn_0='A';//路径低位复位
    char pathCurrentColumn_1='\0';//路径列高位复位
    int flag=0;
    char pathCurrentColumn[3] = {pathCurrentColumn_0,pathCurrentColumn_1,0};
    for(int i=0;i<currentColumn;i++){
        if(pathCurrentColumn_0-'A'>=25){//超过了26
            pathCurrentColumn_1='A';
            pathCurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            pathCurrentColumn_0++;
        }else{
            pathCurrentColumn_1++;
            if(pathCurrentColumn_1-'A'>=26){
                pathCurrentColumn_1='A';
                pathCurrentColumn_0++;
            }
        }
        pathCurrentColumn[0] = pathCurrentColumn_0;
        pathCurrentColumn[1] = pathCurrentColumn_1;
        cellCentreAndText(pathCurrentColumn+QString::number(1),Location[i]);
    }
    ////写入最短距离
    for(int i=0;i<currentColumn;i++){
        pathCurrentColumn_0='A';//路径低位复位
        pathCurrentColumn_1='\0';//路径列高位复位
        flag=0;
        pathCurrentColumn[0] = pathCurrentColumn_0;
        pathCurrentColumn[1] = pathCurrentColumn_1;
        for(int j=0;j<currentColumn;j++){
            if(pathCurrentColumn_0-'A'>=25){//超过了26
                pathCurrentColumn_1='A';
                pathCurrentColumn_0='A';
                flag=1;
            }else if(flag==0){
                pathCurrentColumn_0++;
            }else{
                pathCurrentColumn_1++;
                if(pathCurrentColumn_1-'A'>=26){
                    pathCurrentColumn_1='A';
                    pathCurrentColumn_0++;
                }
            }
            pathCurrentColumn[0] = pathCurrentColumn_0;
            pathCurrentColumn[1] = pathCurrentColumn_1;
            if(dist[i][j]!=INF){//dist[i][j]!=0
                cellCentreAndText(pathCurrentColumn+QString::number(2+i),QString::number(dist[i][j]));
            }//长度INF的情况，不做处理（这种情况是因为到该节点没有通路，长度为无穷）
        }
    }
    ////计算各个节点路径长总和
    //总计文字
    cellCentreAndText("A"+QString::number(currentColumn+2),"总计");
    //统计
    pathCurrentColumn_0='A';//路径低位复位
    pathCurrentColumn_1='\0';//路径列高位复位
    flag=0;
    pathCurrentColumn[0] = pathCurrentColumn_0;
    pathCurrentColumn[1] = pathCurrentColumn_1;
    QString Column;
    unsigned int totalSum;
    for(int i=0;i<currentColumn;i++){//第一行未数
        if(pathCurrentColumn_0-'A'>=25){//超过了26
            pathCurrentColumn_1='A';
            pathCurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            pathCurrentColumn_0++;
        }else{
            pathCurrentColumn_1++;
            if(pathCurrentColumn_1-'A'>=26){
                pathCurrentColumn_1='A';
                pathCurrentColumn_0++;
            }
        }
        pathCurrentColumn[0] = pathCurrentColumn_0;
        pathCurrentColumn[1] = pathCurrentColumn_1;
        nodeCell = sheet->querySubObject("Range(QVariant,QVariant)",pathCurrentColumn+QString::number(currentColumn+2));
        if(i<currentColumn-1){//计算各个节点路径长总和
            Column = pathCurrentColumn;
            nodeCell->dynamicCall("SetValue(conts QVariant&)", QVariant("=SUM(" + Column + QString::number(2) + ":" + Column + QString::number(currentColumn) + ")"));
        }
        else{//计算路径长总和
            nodeCell->dynamicCall("SetValue(conts QVariant&)", QVariant("=SUM(B" + QString::number(currentColumn+2) + ":" + Column + QString::number(currentColumn+2) + ")"));
            totalSum = nodeCell->dynamicCall("Value2()").toInt();
        }
        nodeCell->setProperty("HorizontalAlignment",-4108);//设置居中
        nodeCell->setProperty("VerticalAlignment",-4108);
    }
    ////计算重要性
    //文字
    cellCentreAndText("A"+QString::number(currentColumn+3),"重要性");
    //计算
    pathCurrentColumn_0='A';//路径低位复位
    pathCurrentColumn_1='\0';//路径列高位复位
    flag=0;
    pathCurrentColumn[0] = pathCurrentColumn_0;
    pathCurrentColumn[1] = pathCurrentColumn_1;
    for(int i=0;i<currentColumn-1;i++){//第一行未数
        if(pathCurrentColumn_0-'A'>=25){//超过了26
            pathCurrentColumn_1='A';
            pathCurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            pathCurrentColumn_0++;
        }else{
            pathCurrentColumn_1++;
            if(pathCurrentColumn_1-'A'>=26){
                pathCurrentColumn_1='A';
                pathCurrentColumn_0++;
            }
        }
        pathCurrentColumn[0] = pathCurrentColumn_0;
        pathCurrentColumn[1] = pathCurrentColumn_1;
        Column = pathCurrentColumn;
        cellCentreAndText(pathCurrentColumn+QString::number(currentColumn+3),"=ROUND(IMDIV(" + QString::number(totalSum) + "," + Column + QString::number(currentColumn+2) + ")," + QString::number(num_digits) + ")");
    }
    //操作结束
    saveAndCloseExcel();
}
////输出文件保存
void ChinaMap::on_saveOutput_clicked(){//保存输出到文件
    if(!startButton||!endButton){
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择起点或终点"), QMessageBox::tr("确定"));
        return;
    }
    switchAlgorithm();//清空绘图，防止出错
    //loading
    QDialog loading(this);
    loading.setWindowTitle("加载中");
    loading.setMaximumHeight(1);
    loading.open();
    //执行算法
    MatrixToGarcs(MapRelation,&G);
    if(ui->DIJ->isChecked())
        ShortestPath_DIJ(G,Start,End,toNode,frontNode);
    if(ui->BFS->isChecked())
        ShortestPath_BFS(G,Start,End,toNode,frontNode);
    updateShortestPath();
    //保存
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存输出文件"; //对话框标题
    QString filter="Microsoft Excel工作表(*.xlsx);;Microsoft Excel 97-2003 工作表(*.xls)"; //文件过滤器
    QString fileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".xlsx")||fileName.endsWith(".xls")){
        excleFileName = fileName;
        saveOutput();
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
    //完成
    loading.close();
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("保存输出文件完成"), QMessageBox::tr("确定"));
}

void ChinaMap::saveOutput(){//保存输出到文件
    //打开excel并进行初始化
    initSaveExcel();
    //保存数据部分
    //起点和终点
    cellCentreAndText("A1","起点");
    cellCentreAndText("B1",ui->startInput->text());
    cellCentreAndText("A2","终点");
    cellCentreAndText("B2",ui->endInput->text());
    //中转次数
    cellCentreAndText("A3","中转次数");
    cellCentreAndText("B3",QString::number(frontPath));
    //路径长度
    cellCentreAndText("A4","路径长度");
    cellCentreAndText("B4",QString::number(toNode[End]));
    //模式
    cellCentreAndText("A5","模式");
    nodeCell = sheet->querySubObject("Range(QVariant,QVariant)","B5");
    if(ui->DIJ->isChecked())
        nodeCell->setProperty("Value","最短路线");
    if(ui->BFS->isChecked())
        nodeCell->setProperty("Value","中转次数最少路线");
    //走法
    cellCentreAndText("A8","节点名称");
    cellCentreAndText("A9","长度");
    cellCentreAndText("A10","总计");
    char CurrentColumn_0='A';//路径低位复位
    char CurrentColumn_1='\0';//路径列高位复位
    int flag=0;
    char CurrentColumn[3] = {CurrentColumn_0,CurrentColumn_1,0};
    int num=1;//第x站
    while(frontPath>=0){
        if(CurrentColumn_0-'A'>=25){//超过了26
            CurrentColumn_1='A';
            CurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            CurrentColumn_0++;
        }else{
            CurrentColumn_1++;
            if(CurrentColumn_1-'A'>=26){
                CurrentColumn_1='A';
                CurrentColumn_0++;
            }
        }
        CurrentColumn[0] = CurrentColumn_0;
        CurrentColumn[1] = CurrentColumn_1;
        cellCentreAndText(CurrentColumn+QString::number(7),"第"+QString::number(num)+"站");//第x站
        cellCentreAndText(CurrentColumn+QString::number(8),Location[shortestPath[frontPath]]);//节点名称
        cellCentreAndText(CurrentColumn+QString::number(9),QString::number(MapRelation[shortestPath[frontPath+1]][shortestPath[frontPath]]));//长度
        cellCentreAndText(CurrentColumn+QString::number(10),QString::number(toNode[shortestPath[frontPath]]));//总计
        frontPath--;
        num++;
    }
    //第一步长度设置为0
    nodeCell = sheet->querySubObject("Range(QVariant,QVariant)","B9");
    nodeCell->setProperty("Value",0);
    //操作结束
    saveAndCloseExcel();
}
////保存算法执行过程
void ChinaMap::on_saveProgress_clicked(){
    // TODO 将以下内容做成函数
    if(!startButton||!endButton){//未选择起点终点
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("未选择起点或终点"), QMessageBox::tr("确定"));
        return;
    }
    //loading
    QDialog loading(this);
    loading.setWindowTitle("加载中");
    loading.setMaximumHeight(1);
    loading.open();
    //保存
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存算法执行过程"; //对话框标题
    QString filter="Microsoft Excel工作表(*.xlsx);;Microsoft Excel 97-2003 工作表(*.xls)"; //文件过滤器
    QString fileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if(fileName.endsWith(".xlsx")||fileName.endsWith(".xls")){
        excleFileName = fileName;
        saveProgress();
    }else{
        QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("文件错误"), QMessageBox::tr("确定"));
        return;
    }
    //完成
    loading.close();
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("保存输出文件完成"), QMessageBox::tr("确定"));
}

void ChinaMap::saveProgress(){
    switchAlgorithm();//清空绘图，防止出错
    //打开excel并进行初始化
    initSaveExcel();
    //保存数据部分
    double sumTime=0;//计算算法耗时
    //初始化图
    MatrixToGarcs(MapRelation,&G);
    //打印字
    cellCentreMergeAndText("=(A1:B1)","执行次数/次");
    cellCentreMergeAndText("=(A2:B2)","执行时间/ms");
    cellCentreMergeAndText("=(A3:B3)","执行平均时间/ms	");
    cellCentreMergeAndText("=(E1:F1)","起点");
    cellCentreMergeAndText("=(E2:F2)","终点");
    cellCentreMergeAndText("=(E3:F3)","中转次数");
    cellCentreMergeAndText("=(I1:J1)","起点编号");
    cellCentreMergeAndText("=(I2:J2)","终点编号");
    cellCentreMergeAndText("=(I3:J3)","路径长度");
    cellCentreMergeAndText("=(A5:G5)","前面点数组");
    cellCentreMergeAndText("=(A6:C6)","编号");
    cellCentreMergeAndText("=(A7:C7)","节点编号");
    cellCentreMergeAndText("=(A9:G9)","起点到其余点路径长度数组");
    cellCentreMergeAndText("=(A10:C10)","编号");
    cellCentreMergeAndText("=(A11:C11)","路径长度");
    cellCentreMergeAndText("=(A13:G13)","路径走法数组");
    cellCentreMergeAndText("=(A14:C14)","编号");
    cellCentreMergeAndText("=(A15:C15)","节点编号");
    cellCentreAndText("C1",QString::number(execuseTimes));//执行次数
    cellCentreAndText("G1",Location[Start]);//起点
    cellCentreAndText("G2",Location[End]);//终点
    cellCentreAndText("K1",QString::number(Start));//起点编号
    cellCentreAndText("K2",QString::number(End));//终点编号
    ///DIJ算法部分
    sheet->setProperty("Name","Dijkstra算法计算最短路线");//设置表名
    for(int i=0;i<execuseTimes;i++)//计算时间
        sumTime+=ShortestPath_DIJ(G,Start,End,toNode,frontNode);
    updateShortestPath();
    //写入
    cellCentreAndText("C2","=ROUND(" + QString::number(sumTime) + "," + QString::number(num_digits) + ")");//执行时间
    cellCentreAndText("C3","=ROUND(IMDIV(C2,C1)," + QString::number(num_digits) + ")");//执行平均时间
    cellCentreAndText("G3",QString::number(frontPath));//中转次数
    cellCentreAndText("K3",QString::number(toNode[End]));//路径长度
    char CurrentColumn_0='C';//路径低位复位
    char CurrentColumn_1='\0';//路径列高位复位
    int flag=0;
    char CurrentColumn[3] = {CurrentColumn_0,CurrentColumn_1,0};
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//输出所有的原始节点和创建的节点
        if(CurrentColumn_0-'A'>=25){//超过了26
            CurrentColumn_1='A';
            CurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            CurrentColumn_0++;
        }else{
            CurrentColumn_1++;
            if(CurrentColumn_1-'A'>=26){
                CurrentColumn_1='A';
                CurrentColumn_0++;
            }
        }
        CurrentColumn[0] = CurrentColumn_0;
        CurrentColumn[1] = CurrentColumn_1;
        //前面点数组
        cellCentreAndText(CurrentColumn+QString::number(6),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(7),QString::number(frontNode[i]));//写入节点编号
        //起点到其余点路径长度数组
        cellCentreAndText(CurrentColumn+QString::number(10),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(11),QString::number(toNode[i]));//写入路径长度
        //路径走法数组
        cellCentreAndText(CurrentColumn+QString::number(14),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(15),QString::number(shortestPath[i]));//写入路径长度
    }
    sumTime=0;
    ///BFS算法部分
    appendSheet("BFS计算最少中转次数路线");//加入新工作表
    cellCentreMergeAndText("=(A1:B1)","执行次数/次");
    cellCentreMergeAndText("=(A2:B2)","执行时间/ms");
    cellCentreMergeAndText("=(A3:B3)","执行平均时间/ms	");
    cellCentreMergeAndText("=(E1:F1)","起点");
    cellCentreMergeAndText("=(E2:F2)","终点");
    cellCentreMergeAndText("=(E3:F3)","中转次数");
    cellCentreMergeAndText("=(I1:J1)","起点编号");
    cellCentreMergeAndText("=(I2:J2)","终点编号");
    cellCentreMergeAndText("=(I3:J3)","路径长度");
    cellCentreMergeAndText("=(A5:G5)","前面点数组");
    cellCentreMergeAndText("=(A6:C6)","编号");
    cellCentreMergeAndText("=(A7:C7)","节点编号");
    cellCentreMergeAndText("=(A9:G9)","起点到其余点路径长度数组");
    cellCentreMergeAndText("=(A10:C10)","编号");
    cellCentreMergeAndText("=(A11:C11)","路径长度");
    cellCentreMergeAndText("=(A13:G13)","路径走法数组");
    cellCentreMergeAndText("=(A14:C14)","编号");
    cellCentreMergeAndText("=(A15:C15)","节点编号");
    cellCentreAndText("C1",QString::number(execuseTimes));//执行次数
    cellCentreAndText("G1",Location[Start]);//起点
    cellCentreAndText("G2",Location[End]);//终点
    cellCentreAndText("K1",QString::number(Start));//起点编号
    cellCentreAndText("K2",QString::number(End));//终点编号
    //计算时间
    for(int i=0;i<execuseTimes;i++)//计算时间
        sumTime+=ShortestPath_BFS(G,Start,End,toNode,frontNode);
    updateShortestPath();
    //写入
    cellCentreAndText("C2","=ROUND(" + QString::number(sumTime) + "," + QString::number(num_digits) + ")");//执行时间
    cellCentreAndText("C3","=ROUND(IMDIV(C2,C1)," + QString::number(num_digits) + ")");//执行平均时间
    cellCentreAndText("G3",QString::number(frontPath));//中转次数
    cellCentreAndText("K3",QString::number(toNode[End]));//路径长度
    CurrentColumn_0='C';//路径低位复位
    CurrentColumn_1='\0';//路径列高位复位
    flag=0;
    CurrentColumn[0] = CurrentColumn_0;
    CurrentColumn[1] = CurrentColumn_1;
    CurrentColumn[2] = 0;
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//输出所有的原始节点和创建的节点
        if(CurrentColumn_0-'A'>=25){//超过了26
            CurrentColumn_1='A';
            CurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            CurrentColumn_0++;
        }else{
            CurrentColumn_1++;
            if(CurrentColumn_1-'A'>=26){
                CurrentColumn_1='A';
                CurrentColumn_0++;
            }
        }
        CurrentColumn[0] = CurrentColumn_0;
        CurrentColumn[1] = CurrentColumn_1;
        //前面点数组
        cellCentreAndText(CurrentColumn+QString::number(6),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(7),QString::number(frontNode[i]));//写入节点编号
        //起点到其余点路径长度数组
        cellCentreAndText(CurrentColumn+QString::number(10),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(11),QString::number(toNode[i]));//写入路径长度
        //路径走法数组
        cellCentreAndText(CurrentColumn+QString::number(14),QString::number(i));//写入编号
        cellCentreAndText(CurrentColumn+QString::number(15),QString::number(shortestPath[i]));//写入路径长度
    }
    sumTime=0;
    ///FLOYD算法部分
    appendSheet("Floyd算法计算多源最短路");//加入新工作表
    cellCentreMergeAndText("=(A1:B1)","执行次数/次");
    cellCentreMergeAndText("=(A2:B2)","执行时间/ms");
    cellCentreMergeAndText("=(A3:B3)","执行平均时间/ms	");
    cellCentreMergeAndText("=(A5:C5)","距离数组");
    cellCentreAndText("C1",QString::number(execuseTimes));//执行次数
    //计算时间
    for(int i=0;i<execuseTimes;i++)//计算时间
        sumTime+=ShortestPath_FLOYD(G,dist);
    cellCentreAndText("C2","=ROUND(" + QString::number(sumTime) + "," + QString::number(num_digits) + ")");//执行时间
    cellCentreAndText("C3","=ROUND(IMDIV(C2,C1)," + QString::number(num_digits) + ")");//执行平均时间
    //写入
    CurrentColumn_0='A';//路径低位复位
    CurrentColumn_1='\0';//路径列高位复位
    flag=0;
    CurrentColumn[0] = CurrentColumn_0;
    CurrentColumn[1] = CurrentColumn_1;
    CurrentColumn[2] = 0;
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//写入行节点名称
        if(CurrentColumn_0-'A'>=25){//超过了26
            CurrentColumn_1='A';
            CurrentColumn_0='A';
            flag=1;
        }else if(flag==0){
            CurrentColumn_0++;
        }else{
            CurrentColumn_1++;
            if(CurrentColumn_1-'A'>=26){
                CurrentColumn_1='A';
                CurrentColumn_0++;
            }
        }
        CurrentColumn[0] = CurrentColumn_0;
        CurrentColumn[1] = CurrentColumn_1;
        cellCentreAndText(CurrentColumn+QString::number(6),Location[i]);
    }
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){//写入列节点名称
        cellCentreAndText("A"+QString::number(i+7),Location[i]);
    }
    for(int i=0;i<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;i++){
        CurrentColumn_0='A';//路径低位复位
        CurrentColumn_1='\0';//路径列高位复位
        flag=0;
        CurrentColumn[0] = CurrentColumn_0;
        CurrentColumn[1] = CurrentColumn_1;
        CurrentColumn[2] = 0;
        for(int j=0;j<ORIGINAL_VERTEX_NUM+createdCustomButtonNum;j++){
            if(CurrentColumn_0-'A'>=25){//超过了26
                CurrentColumn_1='A';
                CurrentColumn_0='A';
                flag=1;
            }else if(flag==0){
                CurrentColumn_0++;
            }else{
                CurrentColumn_1++;
                if(CurrentColumn_1-'A'>=26){
                    CurrentColumn_1='A';
                    CurrentColumn_0++;
                }
            }
            CurrentColumn[0] = CurrentColumn_0;
            CurrentColumn[1] = CurrentColumn_1;
            cellCentreAndText(CurrentColumn+QString::number(i+7),QString::number(dist[i][j]));
        }
    }
    //操作结束
    saveAndCloseExcel();
}

void ChinaMap::on_excuseTimes_valueChanged(int arg1){//修改执行次数
    execuseTimes = arg1;
}
////初始化，关闭，基本操作
void ChinaMap::on_decimalDigits_valueChanged(int arg1){//设置小数位数
    num_digits = arg1;
}

void ChinaMap::initSaveExcel(){//保存初始化
    excel = new QAxObject("Excel.Application");
    excel->setProperty("DisplayAlerts",false);//关闭excel弹窗提示
    works = excel->querySubObject("WorkBooks");
    QFileInfo file(excleFileName);
    if(!file.exists()){//文件不存在则新建文件，去掉这行ifelse则会直接新建文件，删除已经存在的文件
        works->dynamicCall("Add");
    }else{//打开已有的文件，在已有文件上修改
        works->dynamicCall("Open(const QString&)",QDir::toNativeSeparators(excleFileName));
    }
    workbook = excel->querySubObject("ActiveWorkBook");
    sheets = workbook->querySubObject("Sheets");
    sheet = workbook->querySubObject("Sheets(int)",1);
}

void ChinaMap::saveAndCloseExcel(){//保存并关闭
    workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(excleFileName));//保存操作
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    delete excel;
}

void ChinaMap::cellCentreMergeAndText(QString address,QString text){//向单元格写入内容并合并居中
    nodeCell = sheet->querySubObject("Range(QVariant&)",QVariant(address));
    nodeCell->setProperty("MergeCells",true);
    nodeCell->setProperty("HorizontalAlignment",-4108);//设置居中
    nodeCell->setProperty("VerticalAlignment",-4108);
    nodeCell->setProperty("Value",text);
}

void ChinaMap::cellCentreAndText(QString address,QString text){//向单元格写入内容并居中
    nodeCell = sheet->querySubObject("Range(QVariant,QVariant)",address);
    nodeCell->setProperty("HorizontalAlignment",-4108);//设置居中
    nodeCell->setProperty("VerticalAlignment",-4108);
    nodeCell->setProperty("Value",text);
}

void ChinaMap::appendSheet(QString name){//加入新工作表
    int sheetCount = sheets->property("Count").toInt();//获取表的数量
    QAxObject *lastSheet = sheets->querySubObject("Item(int)",sheetCount);
    sheets->querySubObject("Add(QVariant)",lastSheet->asVariant());//在最后的表前添加新表
    sheet = sheets->querySubObject("Item(int)",sheetCount);//指向新添加的表
    lastSheet->dynamicCall("Move(QVariant)",sheet->asVariant());//交换添加的表和最后一张表
    sheet->setProperty("Name",name);
}
////配置文件保存
void ChinaMap::setSettings(QString Group,QString key,QString value){
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(Group);
    settings.setValue(key,value);
    settings.endGroup();
}
void ChinaMap::setSettings(QString Group,QString key,QColor value){
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(Group);
    settings.setValue(key,value);
    settings.endGroup();
}
void ChinaMap::setSettings(QString Group,QString key,int value){
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(Group);
    settings.setValue(key,value);
    settings.endGroup();
}
/*-------------------------------------------------------------------文件保存部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------自定义显示部分----------------------------------------------------------------------*/
void ChinaMap::on_ReadThemeSettings_clicked(){//读取设置
    //读取主题
    QSettings themeSettings("config.ini", QSettings::IniFormat);
    themeSettings.beginGroup("Theme");
    DarkMode = themeSettings.value("DarkMode").toBool();
    on_ThemeButton_clicked(DarkMode);
    ui->ThemeButton->setChecked(DarkMode);
    //读取颜色字体设置
    NodeNameFontColor = themeSettings.value("NodeNameFontColor").value<QColor>();//节点名称颜色
    NodeNameFontSize = themeSettings.value("NodeNameFontSize").toInt();//节点名称字体大小
    ui->setNodeNameFontSize->setValue(NodeNameFontSize);
    PathColor = themeSettings.value("PathColor").value<QColor>();//路径颜色
    PathLengthAngle = themeSettings.value("PathLengthAngle").toBool();//路径长度旋转
    ui->PathLengthAngle->setChecked(PathLengthAngle);
    PathLengthFontColor = themeSettings.value("PathLengthColor").value<QColor>();//路径长度颜色
    PathColorProgress = themeSettings.value("PathColorProgress").value<QColor>();//算法演示的路径颜色
    PathLengthFontSize = themeSettings.value("PathLengthFontSize").toInt();//节点名称字体大小
    ui->setPathLengthFontSize->setValue(PathLengthFontSize);
    ResultColor = themeSettings.value("ResultColor").value<QColor>();//结果颜色
    ShowNodeName = themeSettings.value("ShowNodeName").toBool();//显示节点名称
    ui->ShowNodeName->setChecked(ShowNodeName);
    ShowPathLength = themeSettings.value("ShowPathLength").toBool();//显示路径长度
    ui->ShowPathLength->setChecked(ShowPathLength);
    WindowOpacity = themeSettings.value("Opacity").toInt();//透明度
    if(WindowOpacity<10) WindowOpacity = 10;
    on_Opacity_valueChanged(WindowOpacity);
    ui->Opacity->setValue(WindowOpacity);
    themeSettings.endGroup();
    update();
}

//// 云母效果界面部分
void ChinaMap::setMicaEffect(bool isAlt){
    MARGINS margins{ -1, -1, -1, -1 };//四个边框的保持宽度
    ::DwmExtendFrameIntoClientArea(HWND(this->winId()), &margins);
    WINDOWCOMPOSITIONATTRIBDATA winCompAttrData;
    ACCENT_POLICY accentPolicy;
    winCompAttrData.Attribute = WINDOWCOMPOSITIONATTRIB::WCA_ACCENT_POLICY;
    accentPolicy.AccentState = ACCENT_STATE::ACCENT_ENABLE_HOSTBACKDROP;
    SetWindowCompositionAttribute(HWND(this->winId()), &winCompAttrData);
    DWM_SYSTEMBACKDROP_TYPE system_backdrop_type = isAlt ? DWM_SYSTEMBACKDROP_TYPE::DWMSBT_TABBEDWINDOW : DWM_SYSTEMBACKDROP_TYPE::DWMSBT_MAINWINDOW;
    ::DwmSetWindowAttribute((HWND)(this->winId()), DWMWINDOWATTRIBUTE::DWMWA_SYSTEMBACKDROP_TYPE, &system_backdrop_type, sizeof(DWM_SYSTEMBACKDROP_TYPE));
}

bool ChinaMap::on_ThemeButton_clicked(bool checked){//切换黑暗模式
    BOOL value = checked ? TRUE : FALSE; //根据参数确定深色模式的值
    if(checked){
        ui->ThemeButton->setStyleSheet("QRadioButton::indicator {background-color: transparent;border-color: transparent;} QRadioButton{border-image:url(:/icon/src/image/light.png);})");
        QFile darkTheme(":/qss/src/qss/dark.qss");
        darkTheme.open(QIODevice::ReadOnly);
        setStyleSheet(darkTheme.readAll());
        DarkMode = true;
    }else{
        ui->ThemeButton->setStyleSheet("QRadioButton::indicator {background-color: transparent;border-color: transparent;} QRadioButton{border-image:url(:/icon/src/image/dark.png);})");
        QFile lightTheme(":/qss/src/qss/light.qss");
        lightTheme.open(QIODevice::ReadOnly);
        setStyleSheet(lightTheme.readAll());
        DarkMode = false;
    }
    return SUCCEEDED(::DwmSetWindowAttribute(
        (HWND)this->winId(),
        DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
        &value,
        sizeof(BOOL)));// 返回设置或取消深色模式是否成功
}
////云母效果界面部分
void ChinaMap::on_SaveThemeSettings_clicked(){//保存设置
    setSettings("Theme","ShowPathLength",ShowPathLength);
    setSettings("Theme","PathLengthFontSize",PathLengthFontSize);
    setSettings("Theme","PathLengthColor",PathLengthFontColor);
    setSettings("Theme","PathLengthAngle",PathLengthAngle);
    setSettings("Theme","ShowNodeName",ShowNodeName);
    setSettings("Theme","NodeNameFontSize",NodeNameFontSize);
    setSettings("Theme","NodeNameFontColor",NodeNameFontColor);
    setSettings("Theme","PathColor",PathColor);
    setSettings("Theme","ResultColor",ResultColor);
    setSettings("Theme","Opacity",WindowOpacity);
    setSettings("Theme","DarkMode",DarkMode);
    setSettings("Theme","PathColorProgress",PathColorProgress);
}

void ChinaMap::on_ResetThemeSettings_clicked(){//重置设置
    ShowPathLength=false;                           //显示路线长度
    PathLengthAngle=false;                          //路线长度角度
    PathLengthFontSize=10;                          //显示路线字体大小
    PathLengthFontColor=QColor(65,105,225,200);     //显示路线字体颜色
    ShowNodeName=false;                             //显示节点名称
    NodeNameFontSize=10;                            //显示名称字体大小
    NodeNameFontColor=QColor(65,105,225,200);       //显示名称字体颜色
    PathColor=QColor(47,79,79,80);                  //路径颜色
    ResultColor=QColor(255,0,0,250);                //结果颜色
    DarkMode = false;                               //黑暗主题
    WindowOpacity=100;                              //透明度
    PathColorProgress = QColor(255,165,0,150);      //算法演示的路径颜色
    on_SaveThemeSettings_clicked();
    on_ReadThemeSettings_clicked();
}

void ChinaMap::on_AutoLoadThemeSettings_stateChanged(int arg1){//自动读取
    on_SaveThemeSettings_clicked();//保存现有
    QSettings themeSettings("config.ini", QSettings::IniFormat);
    themeSettings.beginGroup("Theme");
    setSettings("Theme","AutoLoadThemeSettings",arg1);
    themeSettings.endGroup();
}

void ChinaMap::on_ShowPathLength_stateChanged(int arg1){//显示路径长度
    ShowPathLength=arg1;//设置全局量
    update();//更新画图区域
    //启用和禁用字体大小、角度和颜色调整
    ui->setPathLengthFontSize->setEnabled(arg1);
    ui->setPathLengthColor->setEnabled(arg1);
    ui->PathLengthAngle->setEnabled(arg1);
    ui->setPathLengthFontSize_Tip->setEnabled(arg1);
}

void ChinaMap::on_setPathLengthFontSize_valueChanged(int arg1){//路径长度字体大小调整
    PathLengthFontSize=arg1;
    update();//更新画图区域
}

void ChinaMap::on_setPathLengthColor_clicked(){//设置路径长度显示字体颜色
    PathLengthFontColor = QColorDialog::getColor(PathLengthFontColor,this);
    update();//更新画图区域
}

void ChinaMap::on_PathLengthAngle_stateChanged(int arg1){//角度显示
    PathLengthAngle=arg1;
    update();
}

void ChinaMap::on_ShowNodeName_stateChanged(int arg1){//显示节点名称
    ShowNodeName = arg1;
    update();
    ui->setNodeNameFontSize->setEnabled(arg1);
    ui->setNodeNameColor->setEnabled(arg1);
    ui->setNodeNameFontSize_Tip->setEnabled(arg1);
}

void ChinaMap::on_setNodeNameFontSize_valueChanged(int arg1){//节点名称字体大小调整
    NodeNameFontSize=arg1;
    update();//更新画图区域
}

void ChinaMap::on_setNodeNameColor_clicked(){//名称颜色
    NodeNameFontColor = QColorDialog::getColor(NodeNameFontColor,this);
    update();//更新画图区域
}

void ChinaMap::on_pathColor_clicked(){//路径颜色
    PathColor = QColorDialog::getColor(PathColor,this);
    update();//更新画图区域
}

void ChinaMap::on_resultColor_clicked(){//结果颜色
    ResultColor = QColorDialog::getColor(ResultColor,this);
    update();//更新画图区域
}

void ChinaMap::on_Opacity_valueChanged(int value){//透明度
    WindowOpacity = value;
    setWindowOpacity(value/100.0);
    ui->opacity_Tip->setText(QString::number(value)+"%");
}

void ChinaMap::on_PathColor_process_clicked(){//算法演示的路径颜色
    PathColorProgress = QColorDialog::getColor(PathColorProgress,this);
    update();//更新画图区域
}

void ChinaMap::on_resetColor_progress_clicked(){//重置算法演示的路径颜色
    PathColorProgress = QColor(255,165,0,150);//算法演示的路径颜色
    update();//更新画图区域
}
/*-------------------------------------------------------------------自定义显示部分----------------------------------------------------------------------*/
/*-------------------------------------------------------------------数据库交互部分----------------------------------------------------------------------*/
void ChinaMap::on_SignIn_clicked(){//注册
    SignIn *w = new SignIn(this);
    w->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);//固定大小
    w->open();
    int receiver = w->exec();
    if(receiver == QDialog::Accepted){
        ////有效性判断
        //输入为空
        QString signId = w->SignID();//获取输入ID
        QString signPassword = getMd5(w->SignPassword());//获取输入密码
        if(signId==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("用户名为空"), QMessageBox::tr("确定"));
            return;
        }
        if(w->SignPassword()==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("密码为空"), QMessageBox::tr("确定"));
            return;
        }
        //数据库中已存在
        QSqlQuery query(db);
        query.prepare("select userid from user where userid = ?");
        query.addBindValue(signId);
        query.exec();
        if(query.next()){//查询到记录
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("用户已存在"), QMessageBox::tr("确定"));
            return;
        }
        ////写入数据库
        query.prepare("INSERT INTO user (userid, userpassword) VALUES(?, ?)");
        query.addBindValue(signId);query.addBindValue(signPassword);
        query.exec();
        QMessageBox::information(this, QObject::tr("完成"), QObject::tr("注册成功"), QMessageBox::tr("确定"));
    }
    delete w;
}

void ChinaMap::on_LogIn_clicked(){//登录
    LogIn *w = new LogIn(this);
    w->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);//固定大小
    w->open();
    int receiver = w->exec();
    if(receiver == QDialog::Accepted){
        ////有效性判断
        //输入为空
        QString logInId = w->LogInID();//获取输入ID
        QString logPassword = getMd5(w->LogInPassword());//获取输入密码
        if(logInId==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("用户名为空"), QMessageBox::tr("确定"));
            return;
        }
        if(w->LogInPassword()==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("密码为空"), QMessageBox::tr("确定"));
            return;
        }
        ///检查密码和用户名
        QSqlQuery query(db);
        query.prepare("select userid from user where userid = ? and userpassword = ?");
        query.addBindValue(logInId);query.addBindValue(logPassword);
        query.exec();
        if(!query.next()){//未查询到记录
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("用户名或密码错误"), QMessageBox::tr("确定"));
            return;
        }
        //登录成功
        userId = logInId;
        ui->userId->setText(logInId);
        updateSuggest();
        //检查自动登录
        if(w->RememberMe()){
            QSettings settings("config.ini", QSettings::IniFormat);
            settings.beginGroup("USER");
            settings.setValue("userId",logInId);
            settings.setValue("password",logPassword);
            settings.setValue("autoLogin",true);
            settings.endGroup();
        }
        //enable按钮
        ui->ClearAccountData->setEnabled(true);
        ui->DeleteAccount->setEnabled(true);
        ui->LogOut->setEnabled(true);
        ui->ChangePassword->setEnabled(true);
        //如果是管理员
        if(userId == "admin"){
            ui->DeleteAccount->setEnabled(false);//禁用删除账号按钮
            QMessageBox::information(this, QObject::tr("完成"), QObject::tr("登录成功，重启应用以显示管理页面"), QMessageBox::tr("确定"));
        }else{
            ui->BottomTab->removeTab(6);//普通用户删除管理员Tab页
            QMessageBox::information(this, QObject::tr("完成"), QObject::tr("登录成功"), QMessageBox::tr("确定"));
        }
    }
    delete w;
}

void ChinaMap::on_ClearAccountData_clicked(){//删除账号数据
    int receiver = QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("将会删除该账号的所有数据，是否继续？"), QMessageBox::tr("取消"), QMessageBox::tr("确定"));
    if(receiver==1){//选择了确定
        QSqlQuery query(db);
        query.prepare("call DeleteUserData(?)");
        query.addBindValue(userId);
        query.exec();
        updateSuggest();
        QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除用户数据成功"), QMessageBox::tr("确定"));
    }
}

void ChinaMap::on_LogOut_clicked(){//退出登录
    ui->userId->setText("未登录");
    ui->suggest->clear();
    ui->SearchContent->clear();
    ui->favoritePlace->setText("登录以查看常看的地点");
    ui->QueryTimes->setText("登录以查看查询次数");
    userId = "";
    //disable按钮
    ui->ClearAccountData->setEnabled(false);
    ui->DeleteAccount->setEnabled(false);
    ui->LogOut->setEnabled(false);
    ui->ChangePassword->setEnabled(false);
    //删除配置账号数据
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup("USER");
    settings.setValue("userId","");
    settings.setValue("password","");
    settings.setValue("autoLogin",false);
    settings.endGroup();
    ui->BottomTab->removeTab(6);//删除管理员Tab页
    QMessageBox::information(this, QObject::tr("完成"), QObject::tr("退出登录成功"), QMessageBox::tr("确定"));
}

void ChinaMap::on_DeleteAccount_clicked(){//删除账号
    int receiver = QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("将会删除该账号及该账号的所有数据，是否继续？"), QMessageBox::tr("取消"), QMessageBox::tr("确定"));
    if(receiver==1){//选择了确定
        QSqlQuery query(db);
        query.prepare("call DeleteUser(?)");
        query.addBindValue(userId);
        query.exec();
        ui->userId->setText("未登录");
        ui->suggest->clear();
        ui->SearchContent->clear();
        ui->favoritePlace->setText("登录以查看常看的地点");
        ui->QueryTimes->setText("登录以查看查询次数");
        userId = "";
        //disable按钮
        ui->ClearAccountData->setEnabled(false);
        ui->DeleteAccount->setEnabled(false);
        ui->LogOut->setEnabled(false);
        ui->ChangePassword->setEnabled(false);
        //删除配置账号数据
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.beginGroup("USER");
        settings.setValue("userId","");
        settings.setValue("password","");
        settings.setValue("autoLogin",false);
        settings.endGroup();
        QMessageBox::information(this, QObject::tr("完成"), QObject::tr("删除账号成功"), QMessageBox::tr("确定"));
    }
}

void ChinaMap::on_ChangePassword_clicked(){//更改密码
    ChangePassword *w = new ChangePassword(this);
    w->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);//固定大小
    w->open();
    int receiver = w->exec();
    if(receiver == QDialog::Accepted){
        ////有效性判断
        //输入为空
        QString OldPassword = getMd5(w->OldPassword());//获取旧密码
        QString NewPassword = getMd5(w->NewPassword());//获取新密码
        if(w->OldPassword()==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("旧密码为空"), QMessageBox::tr("确定"));
            return;
        }
        if(w->NewPassword()==""){
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("新密码为空"), QMessageBox::tr("确定"));
            return;
        }
        ///检查旧密码
        QSqlQuery query(db);
        query.prepare("select userid from user where userid = ? and userpassword = ?");
        query.addBindValue(userId);query.addBindValue(OldPassword);
        query.exec();
        if(!query.next()){//未查询到记录
            QMessageBox::warning(this, QObject::tr("警告"), QObject::tr("密码错误"), QMessageBox::tr("确定"));
            return;
        }
        //修改密码
        query.prepare("update user set userpassword = ? where userid = ?");
        query.addBindValue(NewPassword);query.addBindValue(userId);
        query.exec();
        //如果设置了自动登录，修改配置文件密码
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.beginGroup("USER");
        bool autoLogin = settings.value("autoLogin").toBool();
        if(autoLogin)
            settings.setValue("password",NewPassword);
        settings.endGroup();
        QMessageBox::information(this, QObject::tr("完成"), QObject::tr("修改成功"), QMessageBox::tr("确定"));
    }
    delete w;
}

QString ChinaMap::getMd5(const QString &value){
    QString md5;
    QByteArray bb;
    bb = QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Md5);
    md5.append(bb.toHex());
    return md5;
}

void ChinaMap::updateSuggest(){
    if(userId!=""){
        //更新推荐
        QSqlQuery query(db);
        query.prepare("select message from suggest where place = (select favoritePlace from user where userid = ?)");
        query.addBindValue(userId);
        query.exec();
        if(query.next()){
            ui->suggest->setText(query.value("message").toString());
        }else{
            ui->suggest->setText("没有相关推荐");
        }
        //更新最爱地点
        query.prepare("select favoritePlace from user where userid = ?");
        query.addBindValue(userId);
        query.exec();
        if(query.next()&&query.value("favoritePlace").toString()!=""){
            ui->favoritePlace->setText(query.value("favoritePlace").toString());
        }else{
            ui->favoritePlace->setText("没有最爱的地点");
        }
        //更新搜索次数
        query.prepare("select count(userid) from log where userid = ?");
        query.addBindValue(userId);
        query.exec();
        if(query.next()){
            ui->QueryTimes->setText(query.value("count(userid)").toString());
        }else{
            ui->QueryTimes->setText("还没有搜索过");
        }
        //更新搜索内容
        query.prepare("select startplace,endplace,usedalgorithm from log where userid = ?");
        query.addBindValue(userId);
        query.exec();
        ui->SearchContent->clear();
        QString content="";
        int column=1;
        content.append("<table border='1' cellpadding='5'><tr><td>编号</td><td>起点</td><td>终点</td><td>使用算法</td></tr>");
        while(query.next()){
            content.append("<tr><td>"+ QString::number(column) +"</td><td>"+ query.value("startplace").toString() +"</td><td>"+ query.value("endplace").toString() +"</td><td>"+ query.value("usedalgorithm").toString() +"</td></tr>");
            column++;
        }
        content.append("</table>");
        ui->SearchContent->setText(content);
    }
}
////管理员界面
void ChinaMap::on_showUser_clicked(){//用户显示
    showUser w(db,this);
    w.show();
    ui->showUser->setEnabled(false);//禁用按钮防止崩溃
    w.exec();
    ui->showUser->setEnabled(true);//启用按钮
}

void ChinaMap::on_showLog_clicked(){//日志显示
    showLog w(db,this);
    w.show();
    ui->showLog->setEnabled(false);
    w.exec();
    ui->showLog->setEnabled(true);
}

void ChinaMap::on_showSuggest_clicked(){//推荐显示
    showSuggest w(db,this);
    w.show();
    ui->showSuggest->setEnabled(false);
    w.exec();
    ui->showSuggest->setEnabled(true);
}
/*-------------------------------------------------------------------数据库交互部分----------------------------------------------------------------------*/
////太过复杂：
//自定义节点文件自动读取和恢复（配置文件）
//基于用户的主题文件
////其他
// BUG 取消系统点显示时系统点还会参与计算（这包括floyd展示，算法展示，生成图分析文件）
