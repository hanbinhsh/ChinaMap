#include "chinamap.h"

#include <QApplication>
#include "QMessageBox.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator base;
    if(base.load(":/translation/src/translations/qtbase_zh_CN.qm")){
        a.installTranslator(&base);
    }else{
        QMessageBox::warning(nullptr, QObject::tr("警告"), QObject::tr("加载翻译文件失败，部分界面可能无法汉化显示！"));
    }
    ChinaMap w;
    w.show();
    return a.exec();
}

