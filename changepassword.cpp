#include "changepassword.h"
#include "ui_changepassword.h"

ChangePassword::ChangePassword(QWidget *parent): QDialog(parent), ui(new Ui::ChangePassword){
    ui->setupUi(this);
}

ChangePassword::~ChangePassword(){
    delete ui;
}

QString ChangePassword::OldPassword(){
    return ui->OldPassword_changepassword->text();
}

QString ChangePassword::NewPassword(){
    return ui->NewPassword_changepassword->text();
}
