#ifndef SIGNIN_H
#define SIGNIN_H

#include <QDialog>

namespace Ui {
class SignIn;
}

class SignIn : public QDialog
{
    Q_OBJECT

public:
    explicit SignIn(QWidget *parent = nullptr);
    ~SignIn();
    QString SignPassword();
    QString SignID();

private:
    Ui::SignIn *ui;
};

#endif // SIGNIN_H
