#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "configurer.h"
#include "bdd.h"

namespace Ui {
class Login;
}

class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

private slots:
    void on_btConnexion_clicked();

    void on_lnPassword_returnPressed();
private:
    Ui::Login *ui;

    QString id,mdp;

    //pointeur sur la classe Configurer pour lancer la fenetre principale
    Configurer *config;

    //pointeur sur la classe BDD pour communiquer avec la BDD
    Bdd *pBdd;

    //initialiser la fenetre
    void init();
};


#endif // LOGIN_H
