#include "login.h"
#include "ui_login.h"

LogIn::LogIn(QWidget *parent): QDialog(parent), ui(new Ui::LogIn){
    ui->setupUi(this);
}

LogIn::~LogIn(){
    delete ui;
}

QString LogIn::LogInID(){
    return ui->ID_login->text();
}

QString LogIn::LogInPassword(){
    return ui->Password_login->text();
}

bool LogIn::RememberMe(){
    return ui->RememberMe->isChecked();
}
