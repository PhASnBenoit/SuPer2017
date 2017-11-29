/*********************************************************
 *Entête devant être résent sur tous les modules
 *Version du source : 2014.0
 *2014-10-02 Modif n°00012452 PhA Modification du source
 ********************************************************/

#include "ihm.h"
#include "ui_ihm.h"
#include "contenuonglet.h"
#include "server.h"

/*----------------*
 *  Constructeur  *
 *----------------*/
Ihm::Ihm(Server *server, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Ihm)
{
    ui->setupUi(this);

    //initialiser le pointeur sur la bdd
    pBdd=new Bdd();
    //si bdd inaccessible
    if(pBdd->accesBdd == false){
        QMessageBox::critical(0, tr("Impossible d'ouvrir la BDD."),
                 tr("Impossible d'etablir une connexion avec la Base de Donnees.\n"),
                          QMessageBox::Cancel);
        this->destroy();
    }

    //initialiser le serveur avec celui instancié dans le main
    _server = server;

    //obtention du nombre de vue max
    int vueMax = pBdd->getVueMax();
    //déclaration QList
    QList<T_TupleOnglet> listeTupleO;
    //récupération des infos sur les onglets
    pBdd->getVue(&listeTupleO);

    if(!listeTupleO.empty()){
        for(int i = 0; i < listeTupleO.count() && i < vueMax; i++) {
            int num_vue = listeTupleO.at(i).num_vue;
            QString legende = listeTupleO.at(i).legende;
            QString image = listeTupleO.at(i).image;
            //ajout de l'onglet
            this->ajoutOnglet(num_vue, legende, image);
        }
    }

    //supprimer les deux onglets de base
    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(vueMax);
    //mettre l'onglet de base dans la vue
    ui->tabWidget->setCurrentIndex(0);

    //régler les temps des timer en fonction de la base de données
    int tempoMouv; // ms tempo pour le timer mouvement
    int tempoRec; // ms tempo pour le timer de réception
    int tempoLec; // ms tempo pour timer des lecteurs
    pBdd->getTempo(&tempoMouv, &tempoRec, &tempoLec);
    this->setTempo(tempoMouv, tempoRec, tempoLec);

    //Alarmes pour affichage du plus récent au plus ancien
    curseur = ui->txtAlarme->textCursor(); //Récupération du QTextCursor de la zone de texte
    position = curseur.position(); //Récupération de sa position
    //Alarmes
    connect(ui->btClear, SIGNAL(clicked()), this, SLOT(clearAlarme()));

    ///SERVEUR
    connect(server, SIGNAL(sig_switchedOn()), this, SLOT(server_switchedOn()));
    connect(server, SIGNAL(sig_switchedOff()), this, SLOT(server_switchedOff()));
    connect(server, SIGNAL(sig_switchedOffOnError(QString)), this, SLOT(server_switchedOffOnError(QString)));
    connect(server, SIGNAL(sig_newConnection(const ClientConnection&)), this, SLOT(server_newConnection(const ClientConnection&)));
    ///DANGER/PERTE
    connect(this, SIGNAL(signalHommeEnDanger(QString &)), this, SLOT(hommeEnDanger(QString &)));//réception signal homme en dange
    connect(this, SIGNAL(signalPerteReception(int, int, T_ListeLabel *)), this, SLOT(perteReception(int, int, T_ListeLabel *)));//réception signal perte réceptin
    //connecter le signal trameRecu au slot AfficherTrame pour débug
    connect(this, SIGNAL(sigTrameRecu(QString &,int)),this,SLOT(sltAfficherTrame(QString &,int)));
    //back-end
    connect(ui->pbConfiguration, SIGNAL(clicked()),this,SLOT(sltBackend()));

    //Ajouter deux menus
    menuFichier = menuBar()->addMenu("&Fichier");
    menuEdition = menuBar()->addMenu("&Edition");
    //ajouter fonction on au menu Fichier
    actionOn = new QAction("&Lancer la supervision",this);
    actionOn->setCheckable(true);
    connect(actionOn, SIGNAL(triggered()), this, SLOT(sltOnCom()));
    menuFichier->addAction(actionOn);

    //Ajouter focntion off au menu Fichier
    actionOff = new QAction("&Arrêter la supervision",this);
    connect(actionOff, SIGNAL(triggered()), this, SLOT(sltOffCom()));
    menuFichier->addAction(actionOff);
    //Ajouter fonciton quitter au menu fichier
    actionQuitter = new QAction("&Quitter",this);
    connect(actionQuitter, SIGNAL(triggered()), this, SLOT(close()));
    menuFichier->addAction(actionQuitter);
    //ajouter fonction lancer back-end au menu fichier
    actionBackend = new QAction("&Lancer Configuration",this);
    connect(actionBackend, SIGNAL(triggered()), this,SLOT(sltBackend()));
    menuEdition->addAction(actionBackend);
    tmrDate = new QTimer();
    connect(tmrDate,SIGNAL(timeout()),this,SLOT(sltActualiserDate()));
    tmrDate->start(1000);


    //////////////////////////////////////////////////
    /***               PARTIE CAMERAS             ***/
 //   http1 = new QNetworkAccessManager(this);
 //   connect(http1, SIGNAL(finished(QNetworkReply*)),this,SLOT(recupererImage1(QNetworkReply*)));
 //   /*http2 = new QNetworkAccessManager(this);
 //   connect(http2, SIGNAL(finished(QNetworkReply*)),this,SLOT(recupererImage2(QNetworkReply*)));*/

    /***               AFFICHAGE IMAGE               ***/
 //   pmImage1 = new QPixmap();
    //pmImage2 = new QPixmap();
 //   timer = new QTimer(this);
 //   connect(timer, SIGNAL(timeout()),this,SLOT(choixItem()));

    /***               CLIC SUR IMAGE               ***/
 //   ui->lImage->installEventFilter(this);
 //   ui->lImage->setMouseTracking(true);

    /*ui->lImage2->installEventFilter(this);
    ui->lImage2->setMouseTracking(true);*/

    /***               COMBO-BOX               ***/
 //   connect(ui->comboBox, SIGNAL(activated(QString)),this,SLOT(choixItem()));
 //   this->actuAffichage();

    /*QSqlQuery queryCam("SELECT Num_Camera, Type_Camera FROM cameras");
    queryCam.exec();
    while(queryCam.next()) {
        QString numCam = queryCam.value(0).toString();
        QString typeCam = queryCam.value(1).toString();

        ui->comboBox->addItem(numCam + "  -  " + typeCam);
    }*/

  //  ui->pbStart->setVisible(false);

    //////////////////////////////////////////////////

    ///DEMARRAGE DE LA SUPERVISION
    emit sltOnCom();
}
/*-------------*
 * Destructeur *
 *-------------*/
Ihm::~Ihm()
{
    //déclaration QList
    QList<T_Badge> listeBadge;

    //récupération des infos sur les onglets
    pBdd->badgeExistant(&listeBadge);

    if(!listeBadge.empty()){
        for(int i = 0; i < listeBadge.count(); i++) {
            int num_badge = listeBadge.at(i).numBadge;

            //mettre badge inactif
            pBdd->setBadgePerdu(num_badge);
        }
    }

    //destruction listeLabel
    while (!listeLabel.empty()){
        T_ListeLabel *tll = listeLabel.takeFirst();
        for(int i=0 ; i<MAXONGLETS ; i++){
            for(int j=0 ; j<MAXBADGES ; j++){
                if(tll->labelB[i][j])
                    delete tll->labelB[i][j];
            } // for j
        } // for i
        for(int i=0 ; i<MAXLECTEURS ; i++) {   // 12/11/2017 destruction des timers lecteur
            if(tll->tpsLecteurs[i])
                delete tll->tpsLecteurs[i];
        } // for
        delete tll->tpsMouv;
        delete tll->tpsSens;
        delete tll;
        delete listeLabel.takeFirst();
    }
    //destruction pointeurs
    delete pBdd;
    //destruction ihm
    delete ui;
    //destruction du server
    delete _server;
    //////////////////////////////////////////////////
    /***               PARTIE CAMERAS             ***/
    //////////////////////////////////////////////////
    //delete pmImage1;
    //delete pmImage2;
    //delete http1;
    //delete http2;
    //delete about;
}


/*--------------------------------*
 * Méthode                        *
 * Historiser les alarmes *
 *--------------------------------*/
int Ihm::historiserAlarme(QString alarme)
{
    QFile log("../logEvents.txt");

    // Ouverture / Création du fichier
    if(!log.exists())
    {
         bool test=log.open(QIODevice::WriteOnly | QIODevice::Text);

         if(!test)
         {
             QMessageBox::critical(0,tr("Erreur lors de l'ouverture"),tr("Impossible d'ouvrir le fichier de log"));
             return 0;
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
    flux <<"Le "<< dateActuelle << " à "<< heureActuelle << ":"<<endl<< "       "<<alarme<<endl;
    return 1;
}
/*------------------*
 * SLOT             *
 * Nettoyer Alarmes *
 *------------------*/
void Ihm::clearAlarme(){
    ui->txtAlarme->clear();
}

/*--------------------------------*
 * Méthode                        *
 * Mise en place temps pour Timer *
 *--------------------------------*/
void Ihm::setTempo(int tempoMouv, int tempoRec, int tempoLec){
    this->tempoM = tempoMouv;
    this->tempoR = tempoRec;
    this->tempoL = tempoLec;
}

/*-----------------*
 * SLOT            *
 * Homme en danger *
 *-----------------*/
void Ihm::hommeEnDanger(QString & nom){
    //obtenir date
    QString date = QDateTime::currentDateTime().toString();
    //affichage texte alarme
    ui->txtAlarme->textCursor().insertHtml(date +"<span style=\"color: red\">"+" +ALARME+ "+"</span>"+ nom + " est en danger !"+"<span style=\"color: red\">"+" Aucun mouvement."+"</span>"+"<br>");
    curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
    ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte

    QString alarme("+ALARME+ "+nom + " est en danger ! Aucun mouvement.");
    //historiser dans fichier txt
    historiserAlarme(alarme);
}
/*-----------------*
 * SLOT            *
 * Perte réception *
 *-----------------*/
void Ihm::perteReception(int numBadge, int numZone, T_ListeLabel *tll){
    //obtenir vue(s) en fonction du lecteur
    //déclaration QList
    QList<T_TupleLecteurS> listeTupleL1;
    pBdd->getVueFctLect(numZone, &listeTupleL1);

    for (int i = 0; i < listeTupleL1.count(); i++){
        int num_vue = listeTupleL1.at(i).num_vue;
        //affichage
        tll->labelB[num_vue][numBadge]->setEnabled(true);
        //en fonction de l'état
        //dans tout les cas images sans sens de passage
        if (num_vue == 1){
            //petite image
            pDynamique.labelB[num_vue][numBadge]->setPixmap(QPixmap("ressources/pers_grise.jpg"));
        }else{    //image normale
            pDynamique.labelB[num_vue][numBadge]->setPixmap(QPixmap("ressources/pers_grise.jpg"));
        }
    } //fin for

    //historiser la perte de réception
    QString alarme = "Perte de réception du badge n° ";
    alarme += numBadge;
    alarme += " dans la zone n° ";
    alarme += numZone;
    this->historiserAlarme(alarme);
}


/*-----------------------*
 * SLOT                  *
 * Tag reçu ; pour debug *
 *-----------------------*/
void Ihm::trameRecu(QString trame){
    bool traitement = this->traitementTrame(trame);
    if(!traitement)
        qDebug() << "trame rejet";
}


/*--------------------------------*
 * SLOT                       *
 * Afficher les trames dans l'ihm pour débug *
 *--------------------------------*/
void Ihm::sltAfficherTrame(QString &trame, int num_badge)
{
    switch(num_badge){
    case 1:
        ui->lbBadge1->setText("Badge 1 : "+trame);
        break;
    case 2:
        ui->lbBadge2->setText("Badge 2 : "+trame);
        break;
    case 3:
        ui->lbBadge3->setText("Badge 3 : "+trame);
        break;
    case 4:
        ui->lbBadge4->setText("Badge 4 : "+trame);
        break;
    case 5:
        ui->lbBadge5->setText("Badge 5 : "+trame);
        break;
    }
}
/*--------------------------------*
 * SLOT 12/11/2017                       *
 * Effacer le timer associé au lecteur en cas de changement de lieu
 *--------------------------------*/
void Ihm::onToLecteur()  // 12/11/2017
{
    int b,i;
    bool found = false;
    T_ListeLabel *tll;

    QTimer *t = (QTimer *)this->sender();
    // recherche du timer concerné dans les badges
    for (b=0 ; b<listeLabel.size() ; b++) { // pour chaque badge
        tll = listeLabel.at(b);
        for (i=0 ; i<MAXLECTEURS ; i++) {   // 12/11/2017 destruction du timer d'un lecteur pour un badge
            if (tll->tpsLecteurs[i] == t) {
                delete tll->tpsLecteurs[i];
                tll->tpsLecteurs[i] = 0;
                found = true;
                break;
            } // if existe
        } // for
        if (found) break;
    } // while
    // raz des données pour le lecteur qui ne reçoit plus le badge
    for(int j=0 ; j<MAXVAL ; j++)
        tll->moySens[i][j] = 0;
    tll->indMoy[i] = 0;
    tll->sdp[i] = 0;
    tll->sdpMem[i] = 0;
}

/*----------------------------------*
 * METHODE                          *
 * Traitement de la trame           *
 *----------------------------------*/
bool Ihm::traitementTrame(QString trame){
    //témoin timer affichage but: faire clignoter
    if (ui->lbActivite->isEnabled())
        ui->lbActivite->setEnabled(false);
    else
        ui->lbActivite->setEnabled(true);
//qDebug() << "[1] dans traitement";

    //décodage trame
    QString num_badge, sens, mouvement, num_lecteur;
    //séparation des parties de la trame
    num_badge = trame.mid(3,3); //numéro de badge
    mouvement = trame.mid(6,3); //niveau de mouvement mesuré
    num_lecteur = trame.mid(9,2);   //numéro du lecteur
    sens = trame.mid(1,2);
    int num_badge_i = num_badge.toInt(0,16);
    int sens_i = sens.toInt(0,16);
    int mouvement_i = mouvement.toInt(0,16);
    int num_lecteur_i = num_lecteur.toInt(0,16);

    emit sigTrameRecu(trame,num_badge.toInt());  //signal pour débug

    if(!pBdd->badgeExiste(num_badge_i)){   //si le badge n'existe pas dans la BDD
        //obtenir date
        QString date = QDateTime::currentDateTime().toString();
        ui->txtAlarme->textCursor().insertText(date+"<Erreur><Badge "+QString::number(num_badge_i)+QString::fromUtf8("> Badge inconnu  dans la Base de données\n"));
        curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
        ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte
        QString alarme("<Erreur><Badge "+num_badge+QString::fromUtf8("> Badge inconnu  dans la Base de données\n"));
        historiserAlarme(alarme);
        return false;
    } // if

    //si le badge n'existe pas sur l'IHM
    if(!pDynamique.BadgeActif[num_badge_i]){
        //Historique des événements (log) : nouveau badge
        pBdd->setLog(1, num_badge_i);    //1=nouveau badge
        //historiser l'evenement
        QString nom;
        pBdd->getNomPersonne(num_badge_i,&nom);
        QString alarme = "Nouveau badge n° : ";
        alarme+= QString::number(num_badge_i) ;
        alarme+= " - ";
        alarme+=nom;
        alarme+= " connecté.";
        this->historiserAlarme(alarme);
        tll = new T_ListeLabel();
        for(int i=0 ; i<MAXLECTEURS ; i++){  // POURQUOI POUR TOUS LES LECTEURS ?????
            for(int j=0 ; j<MAXVAL ; j++){
                if (i == num_lecteur_i)    // 12/11/2017
                    tll->moySens[num_lecteur_i][j] = sens_i;
                else
                    tll->moySens[num_lecteur_i][j] = 0;
            } // for j
        } // for i
        for(int i=0 ; i<MAXLECTEURS ; i++){
                if (i == num_lecteur_i) {   // 12/11/2017
                    tll->sdp[i]=sens_i;      //sens de passage
                    tll->sdpMem[i]=sens_i;
                } else {
                    tll->sdp[i]=0;      //sens de passage
                    tll->sdpMem[i]=0;
                } // else
        }
        memset(tll->indMoy, 0, sizeof(tll->indMoy));    //init Ã  0
//qDebug("Init tll OK.");

        //obtenir vue(s) en fonction du lecteur
        //déclaration QList
        QList<T_TupleLecteurS> listeTupleL;
        tll->zone = num_lecteur_i;
        pBdd->getVueFctLect(tll->zone, &listeTupleL);
//qDebug("Obtient vue OK.");
        //récupération des infos dans la liste
        for (int i = 0; i < listeTupleL.count(); i++) {
            int num_vue = listeTupleL.at(i).num_vue;
            //se placer sur l'onglet
            QWidget *onglet;
            onglet = pDynamique.onglet[num_vue];
            //nouveau label dynamique pour un badge
            tll->labelB[num_vue][num_badge_i] = new QLabel(onglet);
            //The object will be deleted when control returns to the event loop.//ps: j'aime bien ce commentaire !
            connect (tll->labelB[num_vue][num_badge_i], SIGNAL(destroyed()), tll->labelB[num_vue][num_badge_i], SLOT(deleteLater()));
            //sauvegarde de ce label dans Dynamique
            pDynamique.labelB[num_vue][num_badge_i] = tll->labelB[num_vue][num_badge_i];
//qDebug() << pDynamique.labelB[num_vue][num_badge_i];
            //réglage par défaut du nouveau badge
            tll->labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/DefaultConfigure.png"));
            tll->labelB[num_vue][num_badge_i]->setGeometry(590, 620, 20, 20); // largeur hauteur à définir
        } // for i

        //met dans la liste le num badge et lecteur reçu par la trame
        tll->numBadge = num_badge_i;        //numéro de badge
        tll->numLecteur = num_lecteur_i;    //numéro de lecteur
        tll->etat = 0;                      //aller
        // réglage du timer associé au mouvement
        tll->tpsMouv = new QTimer(this);                                    //nouveau Timer
        connect(tll->tpsMouv, SIGNAL(timeout()), this, SLOT(timerMouv()));  //connect timeout
        tll->tpsMouv->setSingleShot(true);                                  //un seul temps
        tll->tpsMouv->start(this->tempoM);                                  //débute le timer
        // réglage du timer associé à la réception
        connect(tll->tpsSens, SIGNAL(timeout()), this, SLOT(timerRec()));   //connect timeout
        tll->tpsSens = new QTimer(this);
        tll->tpsSens->setSingleShot(true);
        tll->tpsSens->start(this->tempoR);
        // ajout à la liste mémoire
        listeLabel.append(tll);

        //maintenant le badge existe sur l'IHM donc le sauvegarder
        pDynamique.BadgeActif[num_badge_i] = true;

    } // if existe pas sur IHM

    // rechercher dans listlabel le tll du badge
    for(int i=0; i<listeLabel.size(); i++){
        tll = listeLabel.at(i);
        if(listeLabel.at(i)->numBadge == num_badge_i)
            break;
    }//for
    //sauvegarde numéro lecteur
    tll->numLecteur = num_lecteur_i;

    tll->etat |= MOUV0;   // mouv=0
    //relance du timer si mouvement
    if (mouvement_i > 0 ) {  // si mouvement
        tll->etat &= ~MOUV;    // alarme mouvement
        tll->etat &= ~MOUV0;   // un seul mouvement
        tll->tpsMouv->setSingleShot(true);
        tll->tpsMouv->start(this->tempoM);  //ms
    }
    // réarmer le timer REC, le créer si nouveau lecteur
    tll->etat &= ~REC;
    //  réarmer le timer de réception
    tll->tpsSens->setSingleShot(true);
    tll->tpsSens->start(this->tempoR);

    //déclaration QList
    QList<T_Personne > listePersonne;
    QList<T_Badge> listeBadge;

    //Recherche identité de la personne
    int num_pers = pBdd->badgeIdentite(num_badge_i, &listePersonne, &listeBadge);
    if (num_pers==-1){
        //le badge n'est pas lié avec une personne
        //obtenir date
        QString date = QDateTime::currentDateTime().toString();
        ui->txtAlarme->textCursor().insertText(date+"<Erreur><Badge "+num_badge+QString::fromUtf8("> Badge non lié à une personne\n"));
        curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
        ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte

        QString alarme("<Erreur><Badge "+num_badge+QString::fromUtf8("> Badge non lié à une personne\n"));
        historiserAlarme(alarme);

    } else {
        tll->nom[num_pers] = listePersonne.at(0).nom;
        tll->prenom[num_pers] = listePersonne.at(0).prenom;
        tll->societe[num_pers] = listePersonne.at(0).societe;
        tll->email[num_pers] = listePersonne.at(0).email;
        tll->portable[num_pers]= listePersonne.at(0).portable;
        tll->id_badge[num_pers] = listeBadge.at(0).id_badge;
    }

    // calcul de la moyenne de la sensibilité
    // 12/11/2017  timer lié au lecteur pour calcul des zones
    if (!tll->tpsLecteurs[num_lecteur_i]) { // si timer du lecteur pas créé
        tll->tpsLecteurs[num_lecteur_i] = new QTimer(this);
        connect(tll->tpsLecteurs[num_lecteur_i], SIGNAL(timeout()), this, SLOT(onToLecteur()));
        for (int i=0 ; i<MAXVAL ; i++) {
            tll->moySens[num_lecteur_i][i] = sens_i ;
        } // for i

    }
    tll->tpsLecteurs[num_lecteur_i]->setSingleShot(true);  // relance le timer
    tll->tpsLecteurs[num_lecteur_i]->start(this->tempoL);
    tll->moySens[num_lecteur_i][tll->indMoy[num_lecteur_i]++] = sens_i;
    if (tll->indMoy[num_lecteur_i] == MAXVAL){
        tll->indMoy[num_lecteur_i] = 0;     //indice du tableau de moyenne
    }
    int moy = calculerMoyenne(tll);     //sur MAXVAL valeur
    tll->sdp[num_lecteur_i] = moy;  //mémo pour calcul sens de passage
    moy -= 100;  //  AVOIR POURQUOI !!! SEMBLE NECESSAIRE

    sensDePassage(tll); //maj de zone et du sens de passage de ce badge

    // recherche si lecteur n'est pas connecté
    if (!pBdd->getEtatLect(num_lecteur_i)){
        qDebug("le lecteur n'est pas connecte ?!");
        //obtenir date
        QString date = QDateTime::currentDateTime().toString();
        ui->txtAlarme->textCursor().insertText(date+"<Erreur><Lecteur "+num_lecteur+QString::fromUtf8("> Lecteur non connecté\n"));
        curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
        ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte
        QString alarme("<Erreur><Lecteur "+num_lecteur+QString::fromUtf8("> Lecteur non connecté\n"));
        historiserAlarme(alarme);
        return false;
    }

    //Obtenir les points de la zone en fonction des vues
//qDebug() << "[2] avant placement";
    //obtenir vue(s) en fonction du lecteur
    //déclaration QList
    QList<T_TupleLecteurS> listeTupleL;
    //obtenir la vue en fonction du lecteur
    pBdd->getVueFctLect(tll->zone, &listeTupleL);
    for (int i = 0; i < listeTupleL.count(); i++) {
        int num_vue = listeTupleL.at(i).num_vue;
        pBdd->getPointsZone(num_vue, tll->zone, &tll->ptA, &tll->ptB);
        this->calculerDroite(moy, tll->ptA, tll->ptB, &tll->ptBadge[num_vue]);
        pDynamique.labelB[num_vue][num_badge_i]->setVisible(true);
//qDebug() << "[3] avant état";
        //en fonction de l'état
        switch(tll->etat) {
        case 0:  // ALLER
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_vert.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/haut_vert.jpg"));
            }
            break;
        case 1:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_rouge.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/haut_rouge.jpg"));
            }
            //homme en danger
            emit signalHommeEnDanger(tll->nom[num_pers]);
            //Historique des événements (log) : alarme mouvement
            pBdd->setLog(3, num_badge_i);    //3=alarme mouvement
            break;
        case 2:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 3:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 4:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_vert.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/bas_vert.jpg"));
            }
            break;
        case 5:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_rouge.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/bas_rouge.jpg"));
            }
            //homme en danger
            emit signalHommeEnDanger(tll->nom[num_pers]);
            //Historique des événements (log) : alarme mouvement
            pBdd->setLog(3, num_badge_i);    //3=alarme mouvement
            break;
        case 6:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 7:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 8:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_orange.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/haut_orange.jpg"));
            }
            break;
        case 9:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_rouge.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/haut_rouge.jpg"));
            }
            //homme en danger
            emit signalHommeEnDanger(tll->nom[num_pers]);
            //Historique des événements (log) : alarme mouvement
            pBdd->setLog(3, num_badge_i);    //3=alarme mouvement
            break;
        case 10:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 11:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 12:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_orange.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/bas_orange.jpg"));
            }
            break;
        case 13:
            if (num_vue == 1 || tll->zone == -1){
                //pas de sens de passage
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/pers_rouge.jpg"));
            } else {
                pDynamique.labelB[num_vue][num_badge_i]->setPixmap(QPixmap("ressources/bas_rouge.jpg"));
            }
            //homme en danger
            emit signalHommeEnDanger(tll->nom[num_pers]);
            //Historique des événements (log) : alarme mouvement
            pBdd->setLog(3, num_badge_i);    //3=alarme mouvement
            break;
        case 14:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        case 15:
            pDynamique.labelB[num_vue][num_badge_i]->setEnabled(false);
            break;
        } //fin switch
//qDebug() << "[4] avant affichage";
        //affichage position exacte badge
        if (num_vue==1 && num_pers==1)  //taille petite, pas de décalement
            pDynamique.labelB[num_vue][num_badge_i]->setGeometry(tll->ptBadge[num_vue].x, tll->ptBadge[num_vue].y,20,20);
        else if (num_vue==1 && num_pers!=1) //taile petite, décalement
            pDynamique.labelB[num_vue][num_badge_i]->setGeometry(tll->ptBadge[num_vue].x + (10*num_pers), tll->ptBadge[num_vue].y,20,20);

        else if (num_vue!=1 && num_pers==1) //taille grande, pas de décalement
            pDynamique.labelB[num_vue][num_badge_i]->setGeometry(tll->ptBadge[num_vue].x, tll->ptBadge[num_vue].y,30,20);
        else    //taille grande, décalement
            pDynamique.labelB[num_vue][num_badge_i]->setGeometry(tll->ptBadge[num_vue].x + (20*num_pers), tll->ptBadge[num_vue].y,30,20);

        //affichage identité personne
        if (num_pers != -1) {
            pDynamique.labelB[num_vue][num_badge_i]->setToolTip("Badge n° "+ QString::number(num_badge_i) +" d'identifiant : "+tll->id_badge[num_pers] +" de  "
                                                         + tll->nom[num_pers] +" "  + tll->prenom[num_pers]
                                                         +QString::fromUtf8("\nSociété : ")+ tll->societe[num_pers]
                                                         +QString::fromUtf8("\nPortable : ")+ tll->portable[num_pers]
                                                         +QString::fromUtf8("\nEmail : ")+ tll->email[num_pers]);
        } else { //badge pas affecté
            pDynamique.labelB[num_vue][num_badge_i]->setToolTip(QString::fromUtf8("Badge non affecté à une personne"));
        } // else
    } //fin for

    pBdd->setBadgeActif(num_badge_i);      //le badge n'est pas perdu
    ui->lNoZone->setText(QString::number(tll->zone));
    return true;
}
/*
void Ihm::actuAffichage()
{

    QSqlQuery queryCam("SELECT Num_Camera FROM cameras");
    queryCam.exec();
    while(queryCam.next()) {
        QString numCam = queryCam.value(0).toString();

        ui->comboBox->addItem(numCam);
    }
}
*/
/*---------------------------*
 * Méthode                   *
 * Calcul droite et Position *
 *---------------------------*/
void Ihm::calculerDroite(int sens, T_Point pointA, T_Point pointB, T_Point *pointF)
{
    float dx, dy, a, x, y;

    //pas de calcul, les points correspondent Ã  la droite (uniquement vue 1)
    if ((pointB.x == 0) && (pointB.y == 0)){
        pointF->x = pointA.x;
        pointF->y = pointA.y;
    }
    //sinon calculer position
    else{
        dx = pointB.x - pointA.x;
        dy = pointB.y - pointA.y;
        x = sens*dx/100;  // mise à l'échelle
        a = dy/dx;     // coeff directeur, pas d'ordonnée à l'origine car changement de repère
        y = a*x;   // équation de la droite
        pointF->x = pointA.x + x;
        pointF->y = pointA.y + y;
    }
}

/*------------------------*
 * Méthode                *
 * Calcul sens de passage *
 * Nouvelle méthode du 13/11/2017 *
 *------------------------*/
bool Ihm::sensDePassage(T_ListeLabel *tll)
{
    int sensMonter;
    // Détermination de la zone ---------------------

    // combien de lecteurs lisent le badge
    int nbLect=1;
    if (tll->sdp[tll->numLecteur+1] > 0) nbLect++;
    else if (tll->sdp[tll->numLecteur-1] > 0) nbLect++;

    if (nbLect == 1) { // un seul lecteur
        tll->zone = tll->numLecteur;
        sensMonter = tll->numLecteur+1;
    } else {
        if (tll->sdp[tll->numLecteur+1]>0) {
            tll->zone = (tll->numLecteur+1)*11+1;
            sensMonter = tll->numLecteur+1;
        } else {
            tll->zone = (tll->numLecteur-1)*11+1;
            sensMonter = tll->numLecteur;
        } // else lecteur
    } // else plusieurs lecteurs

    // Détermination du sens de passage -------------------
    if (tll->sdp[sensMonter] < tll->sdpMem[sensMonter])  tll->etat |= AR;
    if (tll->sdp[sensMonter] > tll->sdpMem[sensMonter]) tll->etat &= ~AR;
    tll->sdpMem[tll->numLecteur] = tll->sdp[tll->numLecteur]; // sauvegarde
    return 1;
} // sensDePassage

/*
bool Ihm::sensDePassage(T_ListeLabel *tll)
{
    int sensMonter = pBdd->getSensMonter(tll->numLecteur);

    //sens de montée = rapprochement
    if (sensMonter == 1){
        //RSSI plus petit donc aller
        if (tll->sdp[tll->numLecteur] < tll->sdpMem[tll->numLecteur]){
            tll->etat |= AR;
        }
        //RSSI plus grand donc retour
        if (tll->sdp[tll->numLecteur] > tll->sdpMem[tll->numLecteur]){
            tll->etat &= ~AR;
        }
        tll->sdpMem[tll->numLecteur] = tll->sdp[tll->numLecteur];   //sauvegarde
        //zone
        if (tll->sdp[tll->numLecteur]>0)
            tll->zone = tll->numLecteur;
        return true;
    //sens de montée = éloignement
    } else if (sensMonter == 2){
        //RSSI plus petit donc retour
        if (tll->sdp[tll->numLecteur] < tll->sdpMem[tll->numLecteur]){
            tll->etat |= AR;
            tll->sdpMem[tll->numLecteur] = tll->sdp[tll->numLecteur];   //sauvegarde
        }
        //RSSI plus grand donc aller
        if (tll->sdp[tll->numLecteur] > tll->sdpMem[tll->numLecteur]){
            tll->etat &= ~AR;
            tll->sdpMem[tll->numLecteur] = tll->sdp[tll->numLecteur];   //sauvegarde
        }
        //zone
        if (tll->sdp[tll->numLecteur]>0)
            tll->zone = tll->numLecteur;
        return true;
    //sens de montée = zone contigüe
    } else if (sensMonter == 3){
        //détermination de la zone contigüe
        if (tll->sdp[tll->numLecteur+1]>0)
            tll->zone = tll->numLecteur*11+1;
        if (tll->sdp[tll->numLecteur-1]>0)
            tll->zone = (tll->numLecteur-1)*11+1;
        return true;
    //pas de sens de passage
    }else{
        tll->zone = -1;
        return false;
    }
}
*/
/*---------------------------*
 * Méthode                   *
 * Calcul Moyenne réception  *
 *---------------------------*/
int Ihm::calculerMoyenne(T_ListeLabel *tll){
    // calcul de la moyenne de la sensibilitée
    int sumMoy=0;
    for (int i=0 ; i<MAXVAL ; i++)
        sumMoy += tll->moySens[tll->numLecteur][i];
    return sumMoy / MAXVAL;

}
/*--------------------*
 * Méthode            *
 * Timer de mouvement *
 *--------------------*/
void Ihm::timerMouv(){
    T_ListeLabel *tll;
    int nbB = listeLabel.size();
    for (int i=0 ; i<nbB ; i++) {
        tll = listeLabel.at(i);
        //si Timer n'est pas actif
        if (!tll->tpsMouv->isActive()) {
            tll->etat |= MOUV;   // homme en danger
        }
    }
}
/*--------------------*
 * Méthode            *
 * Timer de réception *
 *--------------------*/
void Ihm::timerRec(){
    T_ListeLabel *tll;
    int nbB = listeLabel.size();
    //parcours des badges
    for (int i=0 ; i<nbB ; i++) {
        tll = listeLabel.at(i);
        if (tll->tpsSens)
            if (!tll->tpsSens->isActive()) {
                //obtenir date
                QString date = QDateTime::currentDateTime().toString();
                //ajouter alarme
                ui->txtAlarme->textCursor().insertHtml(date+"<span style=\"color: orange\">"+" +ALARME+ Perte de réception"+"</span>"+" du badge "+ QString("%1").arg(tll->numBadge,0,16)+"<br>");
                curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
                ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte
                //Historique des événements (log) : perte réception
                pBdd->setLog(2, i); //2=perte de réception
                emit signalPerteReception(tll->numBadge, tll->zone, tll);
                //arrêt du timer de mouvement
                tll->tpsMouv->stop();
                //mise à jour état
                tll->etat |= REC;
                //perte du badge dans BDD
                pBdd->setBadgePerdu(tll->numBadge);
                //mise à jour tableaux pour sens de passage
//                    tll->sdp[1] = 0;  // au lieu de num_lecteur
//                    tll->sdpMem[1] = 0;
            }
    }
}
/*-----------------*
 * SLOT            *
 * Lecteur inconnu *
 *-----------------*/
void Ihm::lecteurInconnu(QString ip){
    //obtenir date
    QString date = QDateTime::currentDateTime().toString();
    //ajout texte Ihm
    ui->txtAlarme->textCursor().insertText(date+QString::fromUtf8("<Erreur> Quelque chose a tenté de se connecter. Son IP: ")+ip+" \n");
    curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
    ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte

    QString alarme(QString::fromUtf8("<Erreur> Quelque chose a tenté de se connecter. Son IP: ")+ip+" \n");
    historiserAlarme(alarme);
}
/*-----------------*
 * SLOT            *
 * Lecteur inactif *
 *-----------------*/
void Ihm::lecteurInactif(int numLecteur){
    //Supprimer le lecteur
    this->suppLecteur(numLecteur);

}

/*---------------------*
 * Méthode             *
 * Suppression Lecteur *
 *---------------------*/
void Ihm::suppLecteur(int numLecteur){
    //message d'avertissement (Alarmes)
    QString numLecteurS = QString::number(numLecteur);
    QString supLecteur = "<Lecteur ";
    supLecteur += numLecteurS;
    supLecteur += QString::fromUtf8("> vient de se déconnecter");
    //obtenir date
    QString date = QDateTime::currentDateTime().toString();
    ui->txtAlarme->textCursor().insertText(date+supLecteur + "\n");
    curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
    ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte

    QString alarme("Lecteur num: "+supLecteur+"supprimé" + "\n");
    historiserAlarme(alarme);

}
/*---------------*
 * SLOT          *
 * Lecteur actif *
 *---------------*/
void Ihm::lecteurActif(Reader Lecteur){

   // qDebug() << "SLOT lecteurActif";
    ClientConnection *cCL;
    //sender retourne l'adresse de l'objet ayant émis le signal
    //utilisé ensuite pour faire les connect
    cCL = (ClientConnection *) this->sender();
   // qDebug() << "le cCL dans le SLOT =" << cCL;

    //obtenir le numéro de lecteur grâce à la classe Reader
    unsigned int numLecteur = Lecteur.number();
    QString addrLecteur = Lecteur.address();

    //déclaration QList
    QList<T_TupleLecteurE> listeTupleLA;

    //récuperer la vue du lecteur
    pBdd->getVuePosFctLect(numLecteur, &listeTupleLA);

    //légende de la vue du lecteur
    QString legende;

    //récuperer lieu et legende du lecteur
    int numLieu = pBdd->getLieuFctLecteur(numLecteur,legende);

    //récupération des infos dans la liste
    if(!listeTupleLA.empty()){
        for(int i = 0; i < listeTupleLA.count(); i++) {
            int num_vue = listeTupleLA.at(i).num_vue;
            int x = listeTupleLA.at(i).x;
            int y = listeTupleLA.at(i).y;
            //ajout d'un lecteur (en dynamique)
            this->ajoutLecteur(numLecteur,addrLecteur,numLieu,legende,num_vue, x, y, cCL);
        }
    }
    //en cas de suppression //indique qui vient de se déconnecter
    AfficheAlarme *aA = new AfficheAlarme(this, numLecteur);
    connect(aA, SIGNAL(signalLecteurInactif(int)), this, SLOT(lecteurInactif(int)));
    connect(cCL, SIGNAL(sig_disconnected()), aA, SLOT(lecteurInactif()));

}
/*---------------*
 * Méthode       *
 * Ajout Lecteur *
 *---------------*/
void Ihm::ajoutLecteur(int numLecteur, QString addLecteur, int numLieu,QString legende, int num_vue, int x, int y, ClientConnection *cCL)
{
    QWidget *onglet;
    onglet = pDynamique.onglet[num_vue];

    //nouveau label dynamique pour mettre l'image correspondant
    QLabel *labelL = new QLabel(onglet);

    //conversion
    QString numLecteurE = QString::number(numLecteur);

    //différente taille d'images utilisées
    if(num_vue == 1){
        labelL->setPixmap(QPixmap("ressources/lecteur_actif_petit.jpg"));
    }else{
        labelL->setPixmap(QPixmap("ressources/lecteur_actif.jpg"));
    }
    labelL->setGeometry(x, y, 15, 42); // largeur hauteur à définir
    labelL->setVisible(true);

    //affichage informations lecteur
    labelL->setToolTip("Lecteur n° "+ numLecteurE+" actif.\n"
                       +"Adresse IP : "+ addLecteur+"\n"
                       +"Lieu n° "+ QString::number(numLieu)
                       +" : "+legende);

    //message d'avertissement (Alarmes)
    if(num_vue == 1)
    {
        QString ajLecteur("<Lecteur " + numLecteurE + QString::fromUtf8(" > vient de se connecter."));
        qDebug() << ajLecteur;
        //obtenir date
        QString date = QDateTime::currentDateTime().toString();
        ui->txtAlarme->textCursor().insertText(date+ajLecteur + "\n");
        curseur.setPosition(position); // Replacement du curseur à l'endroit où il se trouvait
        ui->txtAlarme->setTextCursor(curseur); // Application du curseur à la zone de texte

        //historiser l'evenement
        QString alarme = "Lecteur n° : ";
        alarme+= numLecteurE ;
        alarme+= " ajouté.";
        historiserAlarme(alarme);
    }

    //en cas de suppression
    connect(cCL, SIGNAL(sig_disconnected()), labelL, SLOT(clear()));
    connect(cCL, SIGNAL(sig_disconnected()), labelL, SLOT(deleteLater()));

}
/*-------------------*
 * Méthode           *
 * Ajout vue(onglet) *
 *-------------------*/
void Ihm::ajoutOnglet(int num_vue, QString legende, QString image)
{
    //nouveau onglet dynamique avec légende
    ContenuOnglet *pContenuOnglet = new ContenuOnglet(0, image);
    ui->tabWidget->insertTab(num_vue, pContenuOnglet, legende);

    //sauvegarde du pointeur onglet
    pDynamique.onglet[num_vue] = pContenuOnglet;
  //  qDebug() << "valeur dans la classe" << pDynamique.onglet[num_vue] << endl;

}
///////////////////////DEMARRAGE/ARRET///////////////////////////////////
/*-------------------*
 * SLOT              *
 * Activation du serveur *
 *-------------------*/
void Ihm::sltOnCom()
{
    //checker le bouton on pour indiquer que le serveur est lancé
    actionOn->setChecked(true);

    //bouton nettoyer
    ui->btClear->setEnabled(true);

    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO;

    //met le serveur en ecoute
    _server->switchOn();

    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO << "switchOn() :" ;

    //activer la premiere caméras disponible
    //on_comboBox_activated(0);
}
/*-------------------*
 * SLOT              *
 * Désactivation du serveur *
 *-------------------*/
void Ihm::sltOffCom()
{
    //dechecker le bouton on pour indiquer que le serveur est désactivé
    actionOn->setChecked(false);

    //fermer le serveur
    _server->switchOff();  // = _server.close()

    //Afficher le débug
    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO;

    //fermer toutes les connexions clients existantes
    _server->closeAllClientConnection();   // = clientConnection.close()
}
/*-------------------*
 * SLOT              *
 * Lancement du back-end *
 *-------------------*/
void Ihm::sltBackend()
{
    QFile log("/opt/SuPer_2015/Projet_Config/Projet_Config");

    // Ouverture / Création du fichier
    if(log.exists())
    {
        //lancer l'appli back-end
        QProcess::startDetached("/opt/SuPer_2015/Projet_Config/Projet_Config");

    } else
    {
        QMessageBox::critical(this, "Erreur", "L'erreur suivante est survenue :\n\n Le logiciel de configuration est introuvable.");
    }


}

void Ihm::sltActualiserDate()
{
    //Récuperation de la date actuelle
    QString heureActuelle = QTime::currentTime().toString();
    QString dateActuelle = QDate::currentDate().toString();

    //création de la chaine finale
    QString date("Nous sommes le "+dateActuelle+" et il est "+heureActuelle+".");

    //Affectation de la chaine finale dans un label
    ui->lbDate->setText(date);
}

/////////////////////////////////////////////////////////////

void Ihm::server_switchedOn()
{
    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO << "design";
}

void Ihm::server_switchedOff()
{
    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO << "design";
}

void Ihm::server_switchedOffOnError(QString error)
{
    qDebug() << QThread::currentThreadId() << Q_FUNC_INFO;
    QMessageBox::critical(this, "Erreur", "L'erreur suivante est survenue :\n" + error + ".\n L'écoute du serveur a été stoppée.");
}

void Ihm::server_newConnection(const ClientConnection &cC)
{
    qDebug() << "Un lecteur se connecte" << QThread::currentThreadId() << Q_FUNC_INFO;
    connect(&cC, SIGNAL(sig_isAReader(Reader)), this, SLOT(lecteurActif(Reader))); //lecteur connecté
    connect(&cC, SIGNAL(sig_isNotAReader(QString)), this, SLOT(lecteurInconnu(QString))); //lecteur (ou autre chose) inconnu
    connect(&cC, SIGNAL(sig_frameReceived(QString)), this, SLOT(trameRecu(QString)));  //données tag
}





    //////////////////////////////////////////////////
    /***               PARTIE CAMERAS             ***/
    //////////////////////////////////////////////////

/*

void Ihm::recupererImage1(QNetworkReply *recupImg1)
{
    QByteArray tabImg1;

    tabImg1 = recupImg1->readAll();

    pmImage1->loadFromData(tabImg1);
    ui->lImage->setPixmap(pmImage1->scaled(300,200,Qt::KeepAspectRatio));
    ui->lImage->setGeometry(682,200,300,200);

    pmImage1->save("ressources/001.jpg", "JPG", -1);
}


void Ihm::choixItem()
{
    timer->setInterval(500);
    timer->start();


        QSqlQuery queryChoix("SELECT URL FROM cameras WHERE estActif = 1");
        queryChoix.exec();
        while(queryChoix.next()) {
            http1->get(QNetworkRequest(QUrl(queryChoix.value(0).toString())));
        }
}

*/
    /***               MESSAGE BOX               ***/
/*
bool Ihm::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->lImage && event->type() == QEvent::MouseButtonPress)
    {
         pmImage1->save("ressources/001.jpg", "JPG", -1);

         about= new QMessageBox(this);

         QAbstractButton *quitter;

         about->setWindowTitle("Camera 1");
         about->setIconPixmap(QPixmap("ressources/001.jpg"));
         quitter = about->addButton(tr("Quitter"), QMessageBox::ActionRole);
         quitter->setText("Quitter");
         QTimer *t1 = new QTimer(about);
         t1->setInterval(500);
         connect(t1, SIGNAL(timeout()), this, SLOT(onT2()));
         t1->start();
         about->exec();
         t1->stop();

    }
    return QObject::eventFilter(obj,event);
}
*/
    /***               GRANDE IMAGE               ***/
/*
void Ihm::onT2()
{
    about->setIconPixmap(QPixmap("ressources/001.jpg"));         // Mettre le bon chemin
}

*/
    /***               BOUTON START               ***/
/*
void Ihm::on_pbStart_clicked()
{
    timer->start();

    //Afficher le bouton qui stoppe la caméra
    ui->pbStart->setVisible(false);

    //Rendre invisible ce bouton
    ui->pbStop->setVisible(true);
}
*/
    /***               BOUTON STOP               ***/
/*
void Ihm::on_pbStop_clicked()
{
    timer->stop();

    //Afficher le bouton qui redémarre la caméra
    ui->pbStart->setVisible(true);

    //Rendre invisible ce bouton
    ui->pbStop->setVisible(false);
}
*/
    /***               AFFICHAGE IMAGE AU DEMARRAGE               ***/
/*
void Ihm::on_comboBox_activated(int index)
{
    index++;
    this->choixItem();

    // Recuperation
    QStringList tab;
    tab.clear();
    QString inactif;
    pBdd->setInactif(inactif);
    QString selectedCam = ui->comboBox->currentText();

    tab.append(selectedCam);
    bool choix = pBdd->setChoixCamera(selectedCam);
    if(!choix) {
        // Erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de choisir une camera."),
                             QMessageBox::Ok);
    }
}*/

