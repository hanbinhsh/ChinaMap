#ifndef SHOWLOG_H
#define SHOWLOG_H

#include "qsqldatabase.h"
#include <QDialog>

namespace Ui {
class showLog;
}

class showLog : public QDialog
{
    Q_OBJECT

public:
    explicit showLog(QSqlDatabase dbt,QWidget *parent = nullptr);
    ~showLog();

private slots:
    void on_clearLog_clicked();
    void on_deleteSelected_clicked();

private:
    Ui::showLog *ui;
};

#endif // SHOWLOG_H
