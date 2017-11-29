#ifndef IHM_H
#define IHM_H

#include <QMainWindow>
#include <QtSql>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QLabel>
#include <QToolTip>
#include <QTextCursor>
//#include <QSound>

#include <QAction>

#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include "dynamique.h"
#include "bdd.h"
#include "server.h"
#include "clientconnection.h"
#include "affichealarme.h"

////////////////////////////////
namespace Ui {
class Ihm;
}

class Ihm : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Ihm(Server* server, QWidget *parent = 0);
    ~Ihm();

//    bool eventFilter(QObject *obj, QEvent *event);


private:
    Ui::Ihm *ui;

    //Ihm dynamique (initialisation + visualisation lecteur)
    void ajoutOnglet(int num_vue, QString legende, QString image);
    void ajoutLecteur(int numLecteur, QString addrLecteur,int numLieu,QString legende,int num_vue, int x, int y, ClientConnection *cCL);
    void suppLecteur(int numLecteur);

    //Init nouveau badge
    void ajoutBadge(int numLecteur, int num_vue);

    /*---Config de Super---*/
    int tempoM; // ms tempo pour le timer mouvement
    int tempoR; // ms tempo pour le timer de rÃ©ception
    int tempoL; // ms tempo pour le timer des lecteurs   12/11/2017
    void setTempo(int, int, int tempoLec);

    QList<T_ListeLabel *> listeLabel;

    //historiser alarme
    int historiserAlarme(QString alarme);

    int calculerMoyenne(T_ListeLabel *tll); //calculer moyenne sensibilité
    bool sensDePassage(T_ListeLabel *tll);  //sens de passage
    void calculerDroite(int sens, T_Point pointA, T_Point pointB, T_Point *pointF);    //calcul position exacte

    // la classe Dynamique (sauvegarde de l'affichage dynamique)
    Dynamique pDynamique;
    //pointeur sur la classe BDD
    Bdd *pBdd;
    Server* _server;

    T_ListeLabel *tll;  //pointeur sur structure

    //traitement de la trame
    bool traitementTrame(QString trame);

    //affichage Alarmes
    QTextCursor curseur; //Récupération du QTextCursor de la zone de texte
    int position; //Récupération de sa position

    //MENUBAR
    QMenu *menuFichier;
    QMenu *menuEdition;
    QAction *actionQuitter;
    QAction *actionBackend;
    QAction *actionOn;
    QAction *actionOff;

    QTimer *tmrDate;


    ///////////////////////////////////////////
    /***               CAMERAS             ***/
    ///////////////////////////////////////////

/*
    QPixmap *pmImage1;
    QMessageBox *about;
    QNetworkAccessManager *http1;

    QTimer *timer;

    QList<T_Camera> listeCamera;

    void actuAffichage();
*/

signals:
    void signalHommeEnDanger(QString &);    //alarme homme en danger
    void signalPerteReception(int numBadge, int numLecteur, T_ListeLabel *);    //perte de réception
    void sigTrameRecu(QString &trame,int num_badge);   //emit a chaque reception de trame

public slots:
    //lié à la connexion/déconnexion des lecteurs
    void lecteurActif(Reader Lecteur);
    void lecteurInactif(int);
    void lecteurInconnu(QString);

    //traitement de la trame
    void trameRecu(QString trame);

    //timer mouvement (homme en danger)
    void timerMouv();
    //timer réception (perte de badge)
    void timerRec();

private slots:
    //homme en danger (alarme)
    void hommeEnDanger(QString &);
    //perte de réception
    void perteReception(int, int numZone, T_ListeLabel *);
    //nettoyer Alarme
    void clearAlarme();

    //demarrage
    void sltOnCom();
    //Arret
    void sltOffCom();
    //lancement du back-end
    void sltBackend();

    void server_switchedOn();
    void server_switchedOff();
    void server_switchedOffOnError(QString);
    void server_newConnection(const ClientConnection&);

    void sltAfficherTrame(QString &trame,int num_badge);//déclenché à chaque reception de trame
    void onToLecteur();    // 12/11/2017  slot timer lecteur
    void sltActualiserDate();   //slot pour l'actualisationde la date


    ///////////////////////////////////////////
    /***               CAMERAS             ***/
    ///////////////////////////////////////////
 /*   void onT2();

    void recupererImage1(QNetworkReply *recupImg1);

    void choixItem();

    void on_pbStart_clicked();

    void on_pbStop_clicked();

    void on_comboBox_activated(int index);*/
};

#endif // IHM_H

