#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class LogIn;
}

class LogIn : public QDialog
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();
    QString LogInPassword();
    QString LogInID();
    bool RememberMe();

private:
    Ui::LogIn *ui;
};

#endif // LOGIN_H
