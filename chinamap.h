#ifndef CHINAMAP_H
#define CHINAMAP_H

#include <QRadioButton>
#include <QMainWindow>
#include <QAxObject>
#include <QtSql>

#define CUSTOM_VERTEX_NUM 30                    //可自己创建的节点数
#define ORIGINAL_VERTEX_NUM 34                  //原地图节点数
#define MAX_VERTEX_NUM ORIGINAL_VERTEX_NUM+CUSTOM_VERTEX_NUM     //最大节点数 = 原地图节点数+可自己创建的节点数

QT_BEGIN_NAMESPACE
namespace Ui { class ChinaMap; }
QT_END_NAMESPACE

class ChinaMap : public QMainWindow
{
    Q_OBJECT

public:
    ChinaMap(QWidget *parent = nullptr);
    ~ChinaMap();
    QRadioButton* findButtonByName(QString name);
    QRadioButton* findButton(int index);
    QString findLocation(QRadioButton *btn);
    void showShortestPath(int start,int end);               //画出最短路径并显示长度
    void solveShortestPath_DIJ(int start,int end);
    void paintEvent(QPaintEvent *event);
    int findCityLocation(QRadioButton *btn);
    void showShortestPath();
    void updateShortestPath();
    void drawNextLineSegment();
    void drawNextLine_progressSegment();
    QString getMd5(const QString &value);                   //返回md5加密信息
    void updateSuggest();                                   //更新推荐
    void EnableCustomNodeSettings(bool enable);             //启用关闭自定义节点设置
    int findButtonByName_Int(QString name);                 //中文名寻找按钮下标

private:
    Ui::ChinaMap *ui;
    QTimer *drawLines;                                      //演示定时器
    QTimer *drawLines_progress;                             //算法过程定时器
    QRadioButton *CustomizeButton[CUSTOM_VERTEX_NUM];       //自定义按钮
    QAxObject *excel;                                       //excel必备
    QAxObject *works;
    QAxObject *workbook;
    QAxObject *sheets;
    QAxObject *sheet;
    QAxObject *nodeCell;                                    //单元格操作
    QSqlDatabase db;                                        //数据库

private slots:
    void getcheckbox();
    void SetXPosition();
    void SetYPosition();
    void SetPosition();                                                                     //设置节点位置
    void SetPosition(int x,int y,int nodeIndex);                                            //设置节点位置
    void Update_CustomNodeComboBox();
    void setCustomNodeFromFile(QString fileName);
    void saveCustomNode();                                                                  //保存自定义节点文件
    QString findNameByObjectName(QString ObjName);                                          //通过objectname找到节点中文名
    void saveGraphAnalyseFile();                                                            //保存地图分析文件
    void saveOutput();                                                                      //保存输出到文件
    void saveProgress();                                                                    //保存算法执行过程
    void switchAlgorithm();                                                                 //切换算法时重置绘图区域
    void initSaveExcel();                                                                   //初始化Excel（保存时）
    void saveAndCloseExcel();                                                               //保存文件并关闭Excel
    void cellCentreMergeAndText(QString address,QString text);                              //向单元格写入内容并合并居中
    void cellCentreAndText(QString address,QString text);                                   //向单元格写入内容并居中
    void appendSheet(QString name);                                                         //加入新工作表
    void setSettings(QString Group,QString key,QString value);                              //修改配置文件
    void setSettings(QString Group,QString key,QColor value);                               //修改配置文件
    void setSettings(QString Group,QString key,int value);                                  //修改配置文件
    void setPathLength(int nodeFrom,int nodeTo,int length);                                 //设置两点间的路径长度
    bool NodeRename(int node,QString name);                                                 //设置节点名称(成功返回true)
    void on_ReadThemeSettings_clicked();
    void on_SaveThemeSettings_clicked();
    void on_AutoLoadThemeSettings_stateChanged(int arg1);
    void on_LogIn_clicked();
    void on_SignIn_clicked();
    void on_LogOut_clicked();
    void on_ClearAccountData_clicked();
    void on_DeleteAccount_clicked();
    void on_ChangePassword_clicked();
    void on_ResetThemeSettings_clicked();
    void on_saveProgress_clicked();
    void on_excuseTimes_valueChanged(int arg1);
    void on_copyOutput_clicked();
    void on_decimalDigits_valueChanged(int arg1);
    void on_Opacity_valueChanged(int value);
    void on_saveOutput_clicked();
    void on_setNodeNameFontSize_valueChanged(int arg1);
    void on_setNodeNameColor_clicked();
    void on_pathColor_clicked();
    void on_resultColor_clicked();
    void on_crateGraphAnalyseFile_clicked();
    void on_ShowPathLength_stateChanged(int arg1);
    void on_setPathLengthFontSize_valueChanged(int arg1);
    void on_setPathLengthColor_clicked();
    void on_PathLengthAngle_stateChanged(int arg1);
    void on_PreStep_clicked();
    void on_SaveCustomNodeToFile_clicked();
    void on_solve_clicked();
    void on_clear_clicked();
    bool on_ThemeButton_clicked(bool checked);
    void on_createNode_clicked();
    void on_SetPath_clicked();
    void on_AnimationSpeed_valueChanged(int value);
    void on_NextStep_clicked();
    void on_PlayAnimationByStep_stateChanged(int arg1);
    void on_startInput_textChanged(const QString &arg1);
    void on_endInput_textChanged(const QString &arg1);
    void on_DeleteCustomNode_clicked();
    bool on_SetNodeName_clicked(bool isInit);
    void on_CustomNodeSelect_currentIndexChanged(int index);
    void on_SetLocationComboBox_currentIndexChanged(int index);
    void on_BottomTab_currentChanged(int index);
    void on_ReadCustomNodeFromFile_clicked();
    void on_ShowNodeName_stateChanged(int arg1);                                            //显示节点名称
    void on_SetXPosition_valueChanged(int arg1);
    void on_SetYPosition_valueChanged(int arg1);
    void on_MapNodeSelect_currentIndexChanged(int index);
    void on_SetPath_Map_clicked();
    void on_SetLocationComboBox_Map_currentIndexChanged(int index);
    void on_SetNodeName_Map_clicked();
    void on_Execute_process_clicked();
    void on_AnimationSpeed_process_valueChanged(int value);
    void on_PathColor_process_clicked();
    void on_resetColor_progress_clicked();
    void on_showUser_clicked();
    void on_showLog_clicked();
    void on_showSuggest_clicked();
    void on_enableOriNode_stateChanged(int arg1);
    void setMicaEffect(bool isAlt = false);
    void on_changeMapPhoto_clicked();
};
#endif // CHINAMAP_H
