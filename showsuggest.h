#ifndef SHOWSUGGEST_H
#define SHOWSUGGEST_H

#include "qsqldatabase.h"
#include <QDialog>

namespace Ui {
class showSuggest;
}

class showSuggest : public QDialog
{
    Q_OBJECT

public:
    explicit showSuggest(QSqlDatabase dbt,QWidget *parent = nullptr);
    ~showSuggest();

private slots:
    void on_Output_clicked(const QModelIndex &index);
    void on_updateSelected_clicked();
    void on_addSuggest_clicked();
    void on_deleteSelected_clicked();
    void on_reset_clicked();

private:
    Ui::showSuggest *ui;
};

#endif // SHOWSUGGEST_H
