#ifndef SHOWUSER_H
#define SHOWUSER_H

#include "qsqldatabase.h"
#include <QDialog>

namespace Ui {
class showUser;
}

class showUser : public QDialog
{
    Q_OBJECT

public:
    explicit showUser(QSqlDatabase dbt,QWidget *parent = nullptr);
    ~showUser();

private slots:
    void on_deleteSelected_clicked();
    void on_updateUserID_clicked();
    void on_Output_clicked(const QModelIndex &index);

private:
    Ui::showUser *ui;
};

#endif // SHOWUSER_H
