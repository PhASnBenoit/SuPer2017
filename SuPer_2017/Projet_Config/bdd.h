#ifndef BDD_H
#define BDD_H

#include <QObject>
#include <QtSql>
#include <QList>
#include <QMessageBox>


//////////////////////////////////////////
/*** STRUCTURE pour retour de fonction***/
//////////////////////////////////////////
//fichier log
typedef struct s_Log {
    QString date;
    QString legende;
    QString nom;
    QString prenom;
    QString societe;
    QString login;
    QString numBadge;
} T_Log;

//personne
typedef struct s_Pers {
    QString num_pers;
    QString nom;
    QString prenom;
    QString societe;
    QString dateDebut;
    QString dateFin;
    QString portable;
    QString email;
} T_Personne;

//badge
typedef struct s_Badge {
    QString numBadge;
    QString id_badge;
    QString dateMiseEnService;
    QString dateChangePile;
    QString nom;
    QString prenom;
    QString societe;
    QString dateDebut;
    QString dateFin;
} T_Badge;

//lecteur
typedef struct s_Lecteur {
    QString numLecteur;
    QString numLieuLecteur;
    QString ipLecteur;
} T_Lecteur;

//vue
typedef struct s_Vue {
    QString numVue;
    QString legende;
    QString image;
} T_Vue;

//lieu
typedef struct s_Lieu {
    QString numLieu;
    QString legende;
} T_Lieu;

//zone
typedef struct s_Zone {
    QString numZone;
    QString numLieu;
    QString sensMonter;
    QString legende;
} T_Zone;

//représentation
typedef struct s_Representation {
    QString numVue;
    QString numLieu;
    QString numZone;
    QString x;
    QString y;
    QString xA;
    QString yA;
    QString xB;
    QString yB;
} T_Representation;




////////////////////////////////////
/***            CAMERA          ***/
////////////////////////////////////
typedef struct s_Camera {
    QString Adresse_IP;
    QString URL;
    QString Port_TCP;
    QString Type_Camera;
    QString Num_Camera;
    QString Num_Lieu;
    QString Num_Vue;
} T_Camera;



/////////////////////////////////////////////
/*** CLASSE pour gérer la Base de Données***/
class Bdd : public QObject
{
    Q_OBJECT
public:
    explicit Bdd(QObject *parent = 0);
    ~Bdd();

    bool accesBdd;  //ok ou nok

    //+++ gestion Authentification +++  PARTIE CHAMART
    QList<QString> getLogin();
    QList<QString> getPassword();
    bool setLogin(QString login,QString selectedLogin);
    bool setPassword(QString password,QString selectedLogin);
    QString selectedLogin; //variable pour modifications d'id

    //+++ gestion log +++
    bool getLog(QList<T_Log> *);  //obtenir historique des événements
    bool removeLog();   //suppression historique
    bool setLogConnection(QString login);//sauver les log de la connexion


    //+++ gestion badge +++
    bool getNumBadge(QString *);    //obtenir liste badge existant
    bool getPersonneALier(QList<T_Personne>*);   //obtenir liste personne à lier
    int getNumPersonne(QString);       //obtenir numéro de la personne
    int getNumPers(int num_badge); //Utilise le numéro de badge et renvoie le numéro de personne associé
    bool getBadgeNonActif(QList<T_Badge> *);   //obtenir badge non actifs
    bool getBadgeLies(QList<T_Badge> *);   //obtenir badge lier
    bool setLier(int num_badge, int num_pers); //affectation
    bool setDelier(int num_badge);        //desaffectation un badge à une personne
    bool AddBadge(int numBadge, QString id_badge, QString dateService, QString datePile);//creation d'un badge
    bool setSuppbadge(int num_badge); //supprime un badge
    bool setModBadge(int numBadge, QString id_badge, QString dateService, QString datePile);// modification info badge

    //+++ gestion personnes +++ PARTIE DURAND +++ MODIFIE PAR CHAMART
    bool getPersonneExistant(QList<T_Personne> *);
    bool addPersonne(int num_pers, QString nom, QString prenom, QString portable, QString email, QString societe, QString dateDebut, QString dateFin); // ajouter Personne
    bool setModPersonne(QString selectedNom, QString nom, QString prenom,QString email, QString portable, QString societe, QString dateDebut, QString dateFin); // Modifier personne
    bool setSuppPersonne(QString nom);
    int getNouveauNumPersonne();


    //+++ gestion lecteurs +++ PARTIE DRONNE  +++ MODIFIE PAR CHAMART
    bool getLecteurExistant(QList<T_Lecteur> *); //Liste des lecteurs existants
    bool setLecteur(QString numLecteur, QString numLieuLecteur, QString ipLecteur); // ajouter lecteur
    bool addLecteur(QString numLecteur, QString numLieuLecteur, QString ipLecteur);// modif lecteur
    bool setSuppLecteur(QString numLecteur); // supprimer lecteur



    //+++ gestion configurer SuPer +++
    //--- gestion des vues ---
    bool getVueExistant(QList<T_Vue> *);   //obtenir liste vue existante
    bool setVue(QString numVue, QString legende, QString image); //ajouter vue
    bool addModVue(QString legendeActuelle, QString numVue, QString legende, QString image); //modifier vue
    bool setSuppVue(QString numVue);    //supprimer vue

    //--- gestion des lieux ---
    bool getLieuExistant(QList<T_Lieu> *);  //obtenir liste lieu existant
    bool setLieu(QString numLieu, QString legende); //ajouter lieu
    bool addModLieu(QString legendeActuelle, QString numLieu, QString legende); //modifier lieu
    bool setSuppLieu(QString numLieu);      //supprimer lieu

    //--- gestion des zones ---
    bool getZoneExistant(QList<T_Zone> *);  //obtenir liste zone existante
    bool setZone(QString numZone, int numLieu, QString sensMonter, QString legende);    //ajouter zone
    int getNumLieu(QString);    //obtenir numéro du lieu
    bool addModZone(QString numZone, QString numLieuActuel, QString numLieu, QString sensMonter, QString legende); //modifier zone
    bool setSuppZone(QString numZone);  //supprimer zone

    //--- gestion des positions ---
    QString getLegendeLieu(QString numLieu);    //obtenir legende lieu
    bool getRepresentation(QList<T_Representation> *, QString numVue, QString numZone);  //obtenir représentation si elle existe
    bool setPosition(QString, QString, QString, QString, QString, QString, QString, QString, QString);   //ajouter positions
    bool addModPosition(QString, QString, QString, QString, QString, QString, QString, QString, QString); //modifier positions
    bool getZoneLierVue(QList<T_Representation> *, QString numVue);     //obtenir les zones liées avec cette vue
    bool setSuppPosition(QString numVue, QString numZone);  //supprimer position

    //--- gestion des timers ---
    void getTempo(int *, int *);    //obtenir tempo
    bool setTempo(QString tempoMouv, QString tempoRecep);   //modifier tempo

    ////////////////////////////////////
    /***            CAMERA          ***/
    ////////////////////////////////////
    bool getCameraExistante(QList<T_Camera> *);
    bool addCamera(QString Adresse_IP, QString URL, QString Port_TCP, QString Type_Camera, int Num_Camera, QString Num_Lieu, QString Num_Vue);
    bool setModCamera(QString Adresse_IP, QString URL, QString Port_TCP, QString Type_Camera, QString selectedNum, QString Num_Lieu, QString Num_Vue);
    bool setSuppCamera(QString num);
    int getNouveauNumCamera();
    int getNumCamera(QString camera);

private:
    //pointeurs sur structure
    T_Log * pLog;
    T_Personne * pPersonne;
    T_Badge * pBadge , * pBadgeLier;
//lundi
    T_Vue * pVue;
    T_Lieu * pLieu;
    T_Lecteur * pLecteur;
    T_Zone * pZone;
    T_Representation * pRepresentation;

    T_Camera *pCamera;


    QSqlQuery * query;
    QSqlDatabase  database;
    QString requete;  // requête parfois utilisé
    /*******Décryptage*************/
    QSettings mSettings;
    QByteArray clef ;
    /******************************/

    
signals:

public slots:
    
};

#endif // BDD_H
