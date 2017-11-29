#include "login.h"
#include "ui_login.h"

///////////////
/// MODULE AJOUTE PAR CHAMART 2015
///////////////



Login::Login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    //initialiser le pointeur sur la bdd
    pBdd=new Bdd();

    //initialiser le pointeur sur la classe principale en fournissant la bdd commune
    config=new Configurer(pBdd);

    //si bdd inaccessible
    if(pBdd->accesBdd == false){
        QMessageBox::critical(0, tr("Impossible d'ouvrir la BDD."),
                 tr("Impossible d'etablir une connexion avec la Base de Donnees.\n"),
                          QMessageBox::Cancel);
        this->destroy();
    }


    //initialiser l'ihm
    this->init();
}

Login::~Login()
{
    delete ui;
}

///
/// FONCTION INITIALISER IHM
///
void Login::init()
{
    //initialiser la position de la fenetre
    this->setGeometry(500,250,351,260);

    //nettoyer les lineEdit
    ui->lnLogin->clear();
    ui->lnPassword->clear();

    //masquer le texte saisi dans lnPassword
    ui->lnPassword->setEchoMode(QLineEdit::Password);
}

////
/// SLOT APPUI BOUTON CONNEXION
///
void Login::on_btConnexion_clicked()
{
    QString login,password;

    //recuperer le login saisi
    login= ui->lnLogin->text();

    //Recuperer le mdp saisie
    password=ui->lnPassword->text();

    //obtenir la liste des logins
    QList<QString> listeLogin = pBdd->getLogin();

    //obtenir la liste des password
    QList<QString> listePassword = pBdd->getPassword();

    int i;
    bool res=false;

    //parcourir liste de login
    for(i=0;i<listeLogin.count();i++)
    {
        //si la liste login comporte le login au même index que la liste password comporte le password
        if(listeLogin.at(i) == login && listePassword.at(i) == password)
        {
            //ok
            res =true;

            //sauver la connexion
            pBdd->setLogConnection(login);
        }
    }

    //si ok
    if(res==true)
    {
        //afficher la fenetre de config
        config->show();

        //fermer la fenetre login
        this->close();
    }else
    {
        //reinitialiser l'ihm
        this->init();

        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("L'identifiant ou le mot de passe saisi est incorrect"),
                              QMessageBox::Ok);
    }

    QFile log("../logConnections.txt");
    // Ouverture / CrÃ©ation du fichier
    if(!log.exists())
    {
         bool test=log.open(QIODevice::ReadWrite | QIODevice::Text);

         if(!test)
         {
             QMessageBox::critical(0,tr("Erreur lors de l'ouverture"),tr("Impossible d'ouvrir le fichier"));
             return;
         }
    } else
    {
         log.open(QIODevice::Append | QIODevice::Text);
    }

    //recuperation de la date
    QString heureActuelle = QTime::currentTime().toString();
    QString dateActuelle =QDate::currentDate().toString();


    //attribution du texte
    QTextStream flux(&log);
    //codage
    flux.setCodec("UTF-8");
    //historisation
    flux <<"Le "<< dateActuelle << " a "<< heureActuelle  << "          ID : "<< login <<"           Logiciel : Configuration"<< endl;


}

/////
/// SLOT S'IDENTIFIER AVEC TOUCHE ENTREE
///
void Login::on_lnPassword_returnPressed()
{
    //simuler clic sur btConnexion
    on_btConnexion_clicked();
}
