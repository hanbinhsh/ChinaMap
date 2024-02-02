#include "signin.h"
#include "ui_signin.h"

SignIn::SignIn(QWidget *parent): QDialog(parent), ui(new Ui::SignIn){
    ui->setupUi(this);
}

SignIn::~SignIn(){
    delete ui;
}

QString SignIn::SignID(){
    return ui->ID_sign->text();
}

QString SignIn::SignPassword(){
    return ui->Password_sign->text();
}
