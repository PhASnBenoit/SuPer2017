#include "configurer.h"
#include "ui_configurer.h"

Configurer::Configurer(Bdd *bdd,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Configurer)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->tbConfigBadges->setCurrentIndex(0);
    ui->tbConfigLecteurs->setCurrentIndex(0);
    ui->tbConfigAvancees->setCurrentIndex(0);

    //initialiser la bdd avec celle insatnciee dans Login
    pBdd = bdd;


    this->actuAffichage();      //actualise l'affichage
    this->etatPosition = -1;    //valeur par défaut

    if(pBdd->accesBdd == false){
        QMessageBox::critical(0, tr("Impossible d'ouvrir la BDD."),
                 tr("Impossible d'etablir une connexion avec la Base de Donnees.\n"),
                          QMessageBox::Cancel);
        ui->tabWidget->setEnabled(false);
        ui->btAffichage->setEnabled(false);
    }

    //afficher les historique d'évenement au démarrage
    on_btAffLog_clicked();

    //gestion heure/date
    tmrDate = new QTimer();
    connect(tmrDate,SIGNAL(timeout()),this,SLOT(sltActualiserDate()));
    tmrDate->start(1000);
}

Configurer::~Configurer()
{
    delete pBdd;
    delete ui;
}
/////////////////////////////////////////////////////////////////////////////////////
//méthode permettant l'actualisation de l'affichage
bool Configurer::actuAffichage(){
    ui->label_ConfigModif->setVisible(false);

    //nettoyer champs
    ui->txtInfoPersonne->clear();
    ui->txtInfoBadgeDelier->clear();
    ui->txtInfoBadge->clear();
    ui->txtInfoBadge->clear();
    ui->txtInfoPersonneDelier->clear();

    //nettoyer QList
    listePersonne.clear();
    listeBadge.clear();
    ListeBadgeLies.clear();

    ////////////////////////////////////LOGIN///////////////////////////////////////////////

    //rendre invisible la saisie du mdp
    ui->lnPassword->setEchoMode(QLineEdit::Password);

    //rendre invisible le bouton de confirmation de modification
    ui->pbConfirmerModifLogin->setVisible(false);
    ui->pbVerificationLogin->setVisible(true);

    //affecter des texte par défaut pour les labels
    ui->lbLogin->setText("Identifiant actuel :");
    ui->lbPassword->setText("Mot de passe actuel :");

    ////////////////////////////////////BADGE///////////////////////////////////////////////
    //init personne à lier
    //nettoyer combobox Config badge && affecter/desaffecter
    ui->cBoxModif->clear();
    ui->cBoxSupp->clear();
    ui->cBoxPersonneLier->clear();
    ui->cBoxAffecterBadge->clear();
    ui->cBoxBadgeSupp->clear();
    ui->txtBadgeInfo->setVisible(false);
    ui->label_143->setVisible(false);

    //nettoyer lineedit && textedit
    ui->LeidBadgeCreer->clear();
    ui->LeModifIdBadge->clear();
    ui->LeModifDatePile->clear();
    ui->LeModifDateService->clear();
    ui->LeBadgecreer->clear();
    ui->datePile->clear();
    ui->dateMiseService->clear();
    ui->txtBadgeInfo->clear();

    ui->txtInfoPersonne->clear();
    ui->txtInfoBadge->clear();
    ui->txtInfoBadgeDelier->clear();
    ui->txtInfoPersonneDelier->clear();

    ui->txtInfoBadgeSupp->setReadOnly(true);
    ui->txtInfoBadge->setReadOnly(true);
    ui->txtBadgeInfo->setReadOnly(true);
    ui->txtInfoBadgeDelier->setReadOnly(true);
    ui->txtInfoPersonneDelier->setReadOnly(true);

    //recuperer la liste des badges non actif
    pBdd->getBadgeNonActif(&listeBadge);
    //recuperer la liste des badges liés
    pBdd->getBadgeLies(&ListeBadgeLies);

    //parcourir la liste des badge liés
    if(!ListeBadgeLies.empty()){
        for(int i=0; i<ListeBadgeLies.count(); i++){
            //récuperer le numéro du badge
            QString numBadge = ListeBadgeLies.at(i).numBadge;
            //ajouter le numéro du badge a la combobox suppr
            ui->cBoxBadgeSupp->addItem(numBadge);
            //ajouter le badge à la liste des badges indisponibles
            badgeIndisponibles.append(numBadge);
        }
        //simuler selection du premier item de cbx suppr
    }
    else{
        ui->txtInfoPersonneDelier->setText("Aucun badge à délier");
        ui->txtInfoBadgeDelier->setText("Aucun badge à délier");
        ui->cBoxBadgeSupp->addItem("Aucun badge");
    }

    //parcourir la liste des badge totale
    if(!listeBadge.empty()){
        for(int i=0; i<listeBadge.count(); i++)
        {
            //récuperer le numéro du badge
            QString badge = listeBadge.at(i).numBadge;
            //l'ajouter  au cbx et à la liste des badges indisponibles
            ui->cBoxSupp->addItem(badge);
            ui->cBoxModif->addItem(badge);
            ui->cBoxAffecterBadge->addItem(badge);
            badgeIndisponibles.append(badge);

        }
        //simuler selection du premier item sur les cbx
        on_cBoxAffecterBadge_activated(0);
        on_cBoxModif_activated(0);
        on_cBoxSupp_activated(0);

    }
    else{
        ui->cBoxSupp->addItem("Aucun badge");
        ui->cBoxModif->addItem("Aucun badge");
        ui->cBoxAffecterBadge->addItem("Aucun badge");
        ui->label_ConfigModif->setVisible(true);
        ui->txtInfoBadgeSupp->setText("Veiller à délier le badge pour le supprimer.");
        ui->txtInfoBadge->setText("Aucun badge non lié.");
    }

    //récupérer les personnes à lier
    pBdd->getPersonneALier(&listePersonneALier);

    //parcourir la liste de personne à lier
    if(!listePersonneALier.empty()){
        for(int i = 0; i < listePersonneALier.count(); i++) {
            //récuperer le nom de la personne
            QString nom = listePersonneALier.at(i).nom;
            //ajout du nom dans la cbx
            ui->cBoxPersonneLier->addItem(nom);
        }
    }
    else{
        ui->cBoxPersonneLier->addItem("Aucune personne à lier");
        ui->txtInfoPersonne->setText("Aucune personne n'est à lier.");
    }

    //affichage des badges existant pour rappel dans création d'un badge
    QString badgeExistant;
    //récuperer le numéro des badges existant
    bool retour = pBdd->getNumBadge(&badgeExistant);
    //si aucun badge
    if (retour == false)
        ui->txtBadgeInfo->textCursor().insertText("Aucun badge.");

    //ajouter les numéro de badge la cbx
    ui->txtBadgeInfo->textCursor().insertText(badgeExistant);

    //////////////////////////PERSONNES///////////////////////////////////////////
    //onglet gestion des personnes
    //initialisation
    listePersonne.clear();

    //initialisation
    ui->txtPersonnesExistantes->clear();
    ui->cBoxPersModif->clear();
    ui->cBoxPersSupp->clear();

    //Rendre active les QDate
    ui->ModifierDateDebut->setEnabled(true);
    ui->ModifierDateDebut->setEnabled(true);

    //Rendre in-modifiable les infos personnes
    ui->txtInfoPersDel->setReadOnly(true);
    ui->txtPersonnesExistantes->setReadOnly(true);

    //récuperer la liste des personnes existantes
    pBdd->getPersonneExistant(&listePersonne);

    /************************SERVANT POUR INIT BADGE**********************/
    //Initialisation combobox dans affecter // désaffecter un badge
    //simuler selection du premier nom à lier
    if(!listePersonneALier.empty()){
        on_cBoxPersonneLier_activated(0);
    }

    if(!listePersonne.empty()){
        if(!ListeBadgeLies.empty()){
            on_cBoxBadgeSupp_activated(0);
        }
    }
    /********************************************************************/
    //regler les dateDebutFin par défaut pour pas conserver la derniere ajoutee
    QString sDateDeb("01/01/2015");
    QDate defDateDeb;
    defDateDeb=defDateDeb.fromString(sDateDeb,"dd/MM/yyyy");
    ui->dateDebut->setDate(defDateDeb);
    QString sDateFin("31/12/2015");
    QDate defDateFin;
    defDateFin=defDateFin.fromString(sDateFin,"dd/MM/yyyy");
    ui->dateFin->setDate(defDateFin);

    //parcourir liste des personnes
    if(!listePersonne.empty()){
        for(int i = 0; i < listePersonne.count(); i++){
            QString nom = listePersonne.at(i).nom;
            QString prenom = listePersonne.at(i).prenom;
            QString societe = listePersonne.at(i).societe;
            QString portable = listePersonne.at(i).portable;
            QString email = listePersonne.at(i).email;

            //charger infos personnes existantes
            ui->txtPersonnesExistantes->textCursor().insertText("Nom : " + nom + "\nPrenom : " + prenom + "\nPortable : "+portable+"\nE-mail : "+email+ "\nSociete : " + societe + "\n\n");

            //charger les nom dans la cbxPersModif
            ui->cBoxPersModif->addItem(nom);
            //appeler la fct qui charge les infos persModif
            on_cBoxPersModif_activated(0);

            //charger les nom dans la cbxPersSupp
            ui->cBoxPersSupp->addItem(nom);
            //appeler la fct qui charge les infos persSupp
            on_cBoxPersSupp_activated(0);
        }//for
    }else{
        ui->txtPersonnesExistantes->textCursor().insertText("Aucune Personne");
        ui->cBoxPersModif->addItem("Aucune Personne");
        ui->cBoxPersSupp->addItem("Aucune Personne");
        ui->ModifierDateDebut->setEnabled(false);
        ui->ModifierDateFin->setEnabled(false);
        ui->ModifierPrenomPersonne->setText("Aucune Personne");
        ui->ModifierNomPersonne->setText("Aucune Personne");
        ui->ModifierSociete->setText("Aucune Personne");
    }

       //////////////////////////////LECTEURS//////////////////////////////////////
       //onglet gestion des lecteurs

       //nettoyer QList
       listeLecteur.clear();
       listeLieu.clear();

       ui->lbParametrerLecteur->setVisible(false);
       ui->lbParametrerLecteur_2->setVisible(false);

       //nettoyage zones d'info
       ui->txtInfoLectDel->clear();

       //nettoyer combobox
       ui->cBoxLieuxForLecteur->clear();
       ui->cBoxLieuxForLecteur_Modif->clear();
       ui->cBoxLectSupp->clear();
       ui->cBoxNumLecteursModif->clear();
       ui->txtInfosLieuxForLecteur->clear();
       ui->txtInfosLieuxForLecteur_2->clear();

       //info en readOnly
       ui->txtInfoLectDel->setReadOnly(true);
       ui->txtInfosLieuxForLecteur_2->setReadOnly(true);

       //récup infos lecteurs
       pBdd->getLecteurExistant(&listeLecteur);
       QString numLieuxCouverts;
       //parcourir la liste des lecteurs
       if(!listeLecteur.empty()){
           for(int i = 0; i < listeLecteur.count(); i++){
               //charger num lecteur
               QString numLecteur = listeLecteur.at(i).numLecteur;
               QString ipLecteur = listeLecteur.at(i).ipLecteur;
               QString lieuLecteur = listeLecteur.at(i).numLieuLecteur;

               //charger le num lecteur dans la cbxModif
               ui->cBoxNumLecteursModif->addItem(numLecteur);
               //appeler le slt qui charge les infos du lecteur Ã  modifier
               on_cBoxNumLecteursModif_activated(0);
               //charger le num lecteur dans la cbxSupp
               ui->cBoxLectSupp->addItem(numLecteur);
               //appeler le slt qui charge les infos du lecteur Ã  supprimer
               on_cBoxLectSupp_activated(0);

               //ajout dans la box d'info du badge
               ui->txtInfoLectDel->clear();
               ui->txtInfosLieuxForLecteur_2->textCursor().insertText("Lecteur n° : "+numLecteur+"\nAdresse IP : " + ipLecteur + "\nNuméro du lieu associé : " + lieuLecteur+"\n\n");


               //lieux déja couverts
               numLieuxCouverts += listeLecteur.at(i).numLieuLecteur +"    ";
           }
       }else{
           ui->cBoxNumLecteursModif->addItem("Aucun lecteur");
           ui->lEditIPLecteur_Mod->setText("Aucun lecteur");
           ui->cBoxLieuxForLecteur_Modif->addItem("Aucun lecteur");
           ui->cBoxLectSupp->addItem("Aucun lecteur");
           ui->txtInfoLectDel->setText("Aucun lecteur");
       }


       //recup infos lieux
       pBdd->getLieuExistant(&listeLieu);

       //créer une liste contenant le num des lieux couvert
       QList<QString> lieuxCouverts;
       for(int i=0;i<listeLecteur.count();i++)
       {
           //charger la liste
           lieuxCouverts.append(listeLecteur.at(i).numLieuLecteur);
       }

       //chargement des lieux restant
       if(!listeLieu.empty()){
           for(int i = 0; i < listeLieu.count(); i++){
               QString numLieu = listeLieu.at(i).numLieu;
               QString legende = listeLieu.at(i).legende;

               //Si la liste des lieux couverts ne contient pas ce lieu
               if(lieuxCouverts.contains(numLieu)==false)
               {
                   //Charger le lieu
                   ui->cBoxLieuxForLecteur->addItem(numLieu);
               }
               //charger le num du lieu
               ui->cBoxLieuxForLecteur_Modif->addItem(numLieu);

               //Afficher description lieux
               ui->txtInfosLieuxForLecteur->textCursor().insertText("Lieu " + numLieu + ":" + "\n    Description : " + legende + "\n\n");//onglet ajouter lecteur
           }
       }




       //afficher le num du réseau pour l'IP du lecteur
       ui->lbAdresseReseau->setText("192.168.60.120");
       ui->lEditIPLecteur->setText("192.168.60.");
       ui->lEditIPLecteur->selectAll();



    /// //////////////////////////////CAMERAS////////////////////////////// ///

       // Nettoyage champs
       ui->teInfoCam->clear();

       // Nettoyer QList
       listeCamera.clear();

       // Initialisation
       ui->teResumeCam->clear();
       ui->cb_ModifCam->clear();
       ui->cb_SupprCam->clear();

       // Rendre non modifiable les infos camera
       ui->teInfoCam->setReadOnly(true);
       ui->teResumeCam->setReadOnly(true);

       // Recuperer la liste des cameras existantes
       pBdd->getCameraExistante(&listeCamera);

       // Parcourir liste des cameras
       if(!listeCamera.empty()) {
           for(int i = 0; i < listeCamera.count(); i++) {
               QString Adresse_IP = listeCamera.at(i).Adresse_IP;
               QString URL = listeCamera.at(i).URL;
               QString Port_TCP = listeCamera.at(i).Port_TCP;
               QString Type_Camera = listeCamera.at(i).Type_Camera;
               QString Num_Camera = listeCamera.at(i).Num_Camera;
               QString Num_Lieu = listeCamera.at(i).Num_Lieu;
               QString Num_Vue = listeCamera.at(i).Num_Vue;

               // Charger infos cameras existantes
               ui->teResumeCam->textCursor().insertText("Adresse IP : " + Adresse_IP + "\nURL : " + URL + "\nPort TCP : " + Port_TCP + "\nType Camera : " + Type_Camera + "\nNumero Camera : " + Num_Camera + "\nNumero Lieu : " + Num_Lieu + "\nNumero Vue : " + Num_Vue + "\n\n");

               // Charger les nums dans la cb_ModifCam
               ui->cb_ModifCam->addItem(Num_Camera + "   " + Type_Camera);

               // Charger les nums dans la cb_SupprCam
               ui->cb_SupprCam->addItem(Num_Camera + "   " + Type_Camera);

               // Appeler fonction qui charge les infos SupprCam
               on_cb_SupprCam_activated(0);
           }
       }
       else {
           ui->teResumeCam->textCursor().insertText("Aucune camera");
           ui->cb_ModifCam->addItem("Aucune camera");
           ui->cb_SupprCam->addItem("Aucune camera");
           ui->leModifTypeCam->setText("Aucune camera");
           //ui->leModifNumCam->setText("Aucune camera");
       }

       // Appeler fonction qui charge les infos de modifications de caméra
       on_cb_ModifCam_activated(0);

    //////////////////////////VUES/////////////////////////////////////////
    //onglet gestion des vues (config SuPer)

    //nettoyer QList
    listeVue.clear();

    //init vue existante
    //nettoyer comboBox
    ui->cBoxVueMod->clear();
    ui->cBoxVueSupp->clear();
    ui->cBoxPositionNumVue->clear();
    ui->cBoxPositionSuppNumVue->clear();

    //récupération des infos
    pBdd->getVueExistant(&listeVue);

    if(!listeVue.empty()){
        for(int i = 0; i < listeVue.count(); i++){
            QString numVue = listeVue.at(i).numVue;
            QString legende = listeVue.at(i).legende;

            //ajout du combo
            ui->cBoxVueMod->addItem(legende);
            ui->cBoxVueSupp->addItem(legende);
            ui->cBoxPositionNumVue->addItem(numVue);
            ui->cBoxPositionSuppNumVue->addItem(numVue);
        }
    }

    //////////////////////////LIEUX/////////////////////////////////////////
    //onglet gestion des lieux (config SuPer)
    //nettoyer QList
    listeLieu.clear();

    //init lieu existant
    //nettoyer comboBox
    ui->cBoxLieuMod->clear();
    ui->cBoxLieuSupp->clear();
    ui->cBoxLieuLier->clear();
    ui->cBoxZoneLieuMod->clear();


    //récupération des infos
    //pBdd->getLieuExistant(&listeLieu);

    if(!listeLieu.empty()){
        for(int i = 0; i < listeLieu.count(); i++){
            QString numLieu = listeLieu.at(i).numLieu;
            QString legende = listeLieu.at(i).legende;

            //ajout du combo
            ui->cBoxLieuMod->addItem(legende);
            ui->cBoxLieuSupp->addItem(legende);
            ui->cBoxLieuLier->addItem(legende); //onglet ajouter zone
            ui->cBoxZoneLieuMod->addItem(numLieu);  //onglet modifier zone

        }
    }


    //////////////////////////////ZONES/////////////////////////////////////////
    //onglet gestion des zones (config SuPer)
    //nettoyer QList
    listeZone.clear();

    //init lieu existant
    //nettoyer comboBox
    ui->cBoxZoneMod->clear();
    ui->cBoxZoneSupp->clear();
    ui->cBoxPositionNumZone->clear();

    //récupération des infos
    pBdd->getZoneExistant(&listeZone);

    if(!listeZone.empty()){
        for(int i = 0; i < listeZone.count(); i++){
            QString numZone = listeZone.at(i).numZone;

            //ajout du combo
            ui->cBoxZoneMod->addItem(numZone);
            ui->cBoxZoneSupp->addItem(numZone);
            ui->cBoxPositionNumZone->addItem(numZone);
        }
    }

    ////////////////////////////////TIMERS/////////////////////////////////////////
    //onglet gestion des timers (config SuPer)


    //nettoyer champs
    ui->lEditTimerMouv->clear();
    ui->lEditTimerRecep->clear();

    //récupération des infos
    int i_tempoMouv; // sec tempo pour le timer mouvement
    int i_tempoRecep; // sec tempo pour le timer de réception
    pBdd->getTempo(&i_tempoMouv, &i_tempoRecep);

    //conversion secondes
    float f_tempoMouv = i_tempoMouv;
    float f_tempoRecep = i_tempoRecep;
    f_tempoMouv /= 1000;
    f_tempoRecep /= 1000;

    //ajout des champs
    ui->lEditTimerMouv->insert(QString::number(f_tempoMouv));
    ui->lEditTimerRecep->insert(QString::number(f_tempoRecep));

    //++--++--++//
    return true;
}




////////////////
//bouton actualisation affichage
void Configurer::on_btAffichage_clicked()
{
    bool affichageOk = this->actuAffichage();

    if(affichageOk == true){
        QMessageBox::information(0, tr("Actualiser Affichage"),
                     tr("Interface Actualisee avec succes.\n"),
                              QMessageBox::Ok);
    }else{
        QMessageBox::information(0, tr("Actualiser Affichage"),
                     tr("Erreur : Interface non Actualisee.\n"),
                              QMessageBox::Ok);
    }
}

////////////////////////////////////////SLOTS HISTORIQUE/////////////////////////////////////////////////////////////////////////////
//onglet Historique des événements
//afficher historique des événements

void Configurer::on_btAffLog_clicked()
{
    //déclaration QList
    QList<T_Log> listeLog;

    //supprimer ce qu'il pourrait y avoir à l'écran
    ui->txtLog->clear();

    //récupération de l'historique des événements
    if(!pBdd->getLog(&listeLog)){
        ui->txtLog->insertHtml("<b>L'historique est vide !</b>");
    }

    if(!listeLog.empty()){
        //parcours de la liste
        for(int i =0; i < listeLog.count(); i++){
            QString date_log = listeLog.at(i).date;
            QString legende = listeLog.at(i).legende;
            QString nom = listeLog.at(i).nom;
            QString prenom = listeLog.at(i).prenom;
            QString societe = listeLog.at(i).societe;
            QString login = listeLog.at(i).login;
            int numBadge = listeLog.at(i).numBadge.toInt();
            //si numBadge = 0 alors c'est une alarme de connexion dans SuPer_Configuration
            if(numBadge==0)
            {
                //ajout sur textedit
                ui->txtLog->textCursor().insertHtml("<b>" +date_log+ "</b>" + " | " + "<font color='red'>"+ legende + "</font>" +  " || Login: " + login + "<br>");
            }else
            {
                //ajout sur textedit
                ui->txtLog->textCursor().insertHtml("<b>" +date_log+ "</b>" + " | " + "<font color='red'>"+ legende + "</font>" +  " || Nom: " + nom + " || Prénom: "+ prenom + " || Société: " + societe +"<br>");
            }
        }//for
    }//si non-vide
}

//supprimer historique des événements
void Configurer::on_btSuppLog_clicked()
{
    int clic = QMessageBox::warning(0, tr("Attention : suppression des donnees"),
             tr("Etes vous sur de vouloir supprimer l'Historique ?\n"),
                      QMessageBox::Ok ,QMessageBox::Cancel);

    if(clic == QMessageBox::Ok){
        //requete de suppression
         if(!pBdd->removeLog()){
             ui->txtLog->clear();
             ui->txtLog->insertHtml("<b> <ERREUR> Impossible de supprimer l'Historique </b>");
         }else{
             ui->txtLog->clear();
             ui->txtLog->insertHtml("<b> Operation reussie ! </b>");
        }
    }
}

/////////////////////////////////////SLOT LIER BADGE/////////////////////////////////////////////////////////
//onglet Affecter/désaffecter badge
//Boutons annuler
void Configurer::on_btAnnulerLier_clicked()
{
    //nettoyer et remettre par défaut
    ui->txtBadgeInfo->clear();
    ui->txtInfoPersonne->clear();
    ui->cBoxPersonneLier->setCurrentIndex(0);
    ui->cBoxAffecterBadge->setCurrentIndex(0);
}

void Configurer::on_btAnnulerDelier_clicked()
{
    //nettoyer et remettre par défaut
    ui->txtInfoBadgeDelier->clear();
    ui->txtInfoPersonneDelier->clear();
}


/////////
//SLOT comboBox Lier Personne
void Configurer::on_cBoxPersonneLier_activated(int index)
{
    index=0;
    index++;
    QString nom, prenom, societe, dateDebut, dateFin;
    QString personne = ui->cBoxPersonneLier->currentText();
    int numPersonne =pBdd->getNumPersonne(personne);
    QString sNumPersonne = QString::number(numPersonne);
    for(int i=0; i<listePersonne.count(); i++)
    {
        if(listePersonne.at(i).num_pers == sNumPersonne)
        {
            nom = listePersonne.at(i).nom;
            prenom = listePersonne.at(i).prenom;
            societe = listePersonne.at(i).societe;
            dateDebut = listePersonne.at(i).dateDebut;
            dateFin = listePersonne.at(i).dateFin;
        }
    }

    //ajout info txtInfoPersonne
    //selon la personne (d'où le slot)
    ui->txtInfoPersonne->clear();
    ui->txtInfoPersonne->textCursor().insertText(nom+" "+prenom+" de la société "+societe+". \nDate d'intervention : du "+dateDebut+" au "+dateFin+".");

}
////////
//SLOT bouton de confirmation
void Configurer::on_btOkLier_clicked()
{

    //récupération des informations dans les champs
    int numBadge = ui->cBoxAffecterBadge->currentText().toInt();
    //obtenir le numéro de la personne
    QString personne = ui->cBoxPersonneLier->currentText();
     int numPersonne = pBdd->getNumPersonne(personne);


    if(numPersonne == -1){  //Problème
        QMessageBox::warning(0, tr("Attention : erreur"),
                     tr("Impossible de trouver cette personne.\nVerifier les champs.\nErreur 001."),
                              QMessageBox::Ok);
    }

    //requête
    bool lier = pBdd->setLier(numBadge, numPersonne);

    if(!lier){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                     tr("Impossible de lier cette personne a ce badge.\nVerifier les champs.\nErreur 002."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Lier un Badge a une Personne"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->on_btAnnulerLier_clicked();

        //supprimer combo
        int indexEnCours = ui->cBoxPersonneLier->currentIndex();
        ui->cBoxPersonneLier->removeItem(indexEnCours);

        //nettoyer QList
        listePersonne.clear();

        //init personne à lier
        //récupération des infos
        pBdd->getPersonneALier(&listePersonne);

        if(!listePersonne.empty()){
            for(int i = 0; i < listePersonne.count(); i++) {
                QString nom = listePersonne.at(i).nom;

                //ajout du combo
                ui->cBoxPersonneLier->addItem(nom);

            }
        }
        this->actuAffichage();
    }
}
//////////
////////////////////SLOT BADGE////////////////////////////////////////////////////////////////////
//SLOT ComboBox suppression
void Configurer::on_cBoxBadgeSupp_activated(int index)
{
    /*     modif le 03-02-15     */

    QString num_badge = ui->cBoxBadgeSupp->currentText();
    QString dateMiseEnService = ListeBadgeLies.at(index).dateMiseEnService;
    QString dateChangePile = ListeBadgeLies.at(index).dateChangePile;

    QString num_pers = QString::number(pBdd->getNumPers(num_badge.toInt()));

    QString nom,prenom,societe,dateDebut,dateFin;

    for(int i=0;i<listePersonne.count();i++)
    {
        if(listePersonne.at(i).num_pers==num_pers)
        {
            nom = listePersonne.at(i).nom;
            prenom = listePersonne.at(i).prenom;
            societe = listePersonne.at(i).societe;
            dateDebut = listePersonne.at(i).dateDebut;
            dateFin = listePersonne.at(i).dateFin;
        }
    }


    //ajout info txtBadgeDelier
    ui->txtInfoBadgeDelier->clear();
    ui->txtInfoBadgeDelier->textCursor().insertText("Numéro de Badge : "+num_badge+" \nDate de mise en service : "+dateMiseEnService+" \nDate de changement de pile : "+dateChangePile);
    //ajout info txtInfoPersDelier
    ui->txtInfoPersonneDelier->clear();
    ui->txtInfoPersonneDelier->textCursor().insertText(nom+" "+prenom+" de la sociéte "+societe+". \nDate d'intervention : du "+dateDebut+" au "+dateFin+".");


}
////////
//SLOT bouton de confirmation Suppresion
void Configurer::on_btOkDelier_clicked()
{
    //récupération des informations dans les champs
    int num_badge = ui->cBoxBadgeSupp->currentText().toInt();

    if(num_badge == -1){  //Problème
        QMessageBox::warning(0, tr("Attention : erreur"),
                     tr("Impossible de trouver cette personne.\nVerifier les champs.\nErreur 001c."),
                              QMessageBox::Ok);
    }

    //requête
    bool delier = pBdd->setDelier(num_badge);

    if(!delier){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer ce badge.\nVerifier les champs.\nErreur 002c."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Desaffecter un Badge a une Personne"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->actuAffichage();
    }
}

/*creation badge*/

void Configurer::on_pbAnnulerCreer_clicked()
{
    ui->LeBadgecreer->clear();
    ui->datePile->clear();
    ui->dateMiseService->clear();
    ui->txtBadgeInfo->clear();
}

void Configurer::on_pbConfirmerCreer_clicked()
{
    bool lier;
    bool EtatLier=true;



    //modifier formatDate pour respecter BDD
    ui->dateMiseService->setDisplayFormat("yyyy-MM-dd");
    ui->datePile->setDisplayFormat("yyyy-MM-dd");

    QString num_Badge = ui->LeBadgecreer->text();
    QString dateMiseService = ui->dateMiseService->text();
    QString datePile = ui->datePile->text();
    QString identifiant = ui->LeidBadgeCreer->text();
    int numBadge = num_Badge.toInt();

    //modifier formatDate pour affichage FR
    ui->dateMiseService->setDisplayFormat("dd/MM/yyyy");
    ui->datePile->setDisplayFormat("dd/MM/yyyy");

    if(badgeIndisponibles.contains(num_Badge)){
    ui->txtBadgeInfo->setVisible(true);
    ui->label_143->setVisible(true);
    ui->LeBadgecreer->clear();
    ui->LeidBadgeCreer->clear();
    }

    if(EtatLier==true)
    {
        //requête
        lier = pBdd->AddBadge(numBadge, identifiant, dateMiseService, datePile);
    }
    if(!lier){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
         tr("Impossible de créer ce badge.\nVerifier les champs.\nErreur 002."),
                  QMessageBox::Ok);

    }else{
        QMessageBox::information(0, tr("Badge créer"),
         tr("Operation reussie.\n"),
                  QMessageBox::Ok);
        this->actuAffichage();

    }//if

}

void Configurer::on_cBoxSupp_activated(int index)
{
    QString num_badge = listeBadge.at(index).numBadge;
    QString id_badge = listeBadge.at(index).id_badge;
    QString dateMiseEnService = listeBadge.at(index).dateMiseEnService;
    QString dateChangePile = listeBadge.at(index).dateChangePile;

    //ajout info BadgeDelier
    ui->txtInfoBadgeSupp->clear();
    ui->txtInfoBadgeSupp->textCursor().insertText("Numéro de Badge : "+num_badge+". Identifiant du badge : "+id_badge+". Date de mise en service : "+dateMiseEnService+". Date de changement Pile : "+dateChangePile);

}


void Configurer::on_pbAnnulerSupp_clicked()
{
    this->actuAffichage();
    }

    void Configurer::on_pbConfirmSupp_clicked()
    {

    int num_badge = ui->cBoxSupp->currentText().toInt();
    bool supp = pBdd->setSuppbadge(num_badge);
    if(!supp){
    //erreur
    QMessageBox::warning(0, tr("Attention : requete impossible"),
    tr("Impossible de supprimer ce badge.\nVerifier les champs.\nErreur 002."),
           QMessageBox::Ok);

    }else{
    QMessageBox::information(0, tr("Supprmier un Badge"),
    tr("Operation reussie.\n"),
           QMessageBox::Ok);
    this->actuAffichage();

    }//if


}


void Configurer::on_cBoxModif_activated(int index)
{
    QString dateMiseEnService = listeBadge.at(index).dateMiseEnService;
    QString dateChangePile = listeBadge.at(index).dateChangePile;
    QString id_badge = listeBadge.at(index).id_badge;

    //convertir les string en QDate pour affichage
    QDate dateService;
    dateService=dateService.fromString(dateMiseEnService,"yyyy-MM-dd");
    QDate datePile;
    datePile=datePile.fromString(dateChangePile,"yyyy-MM-dd");


    ui->LeModifDatePile->clear();
    ui->LeModifDateService->clear();
    ui->LeModifIdBadge->clear();

    ui->LeModifIdBadge->setText(id_badge);
    ui->LeModifDatePile->setDate(datePile);
    ui->LeModifDateService->setDate(dateService);
}

void Configurer::on_pushButton_clicked()
{
    ui->LeModifDatePile->clear();
    ui->LeModifDateService->clear();
    ui->cBoxModif->setCurrentIndex(0);
}

void Configurer::on_pushButton_3_clicked()
{

    bool modifier;
    bool EtatLier=true;

    //modifier formatDate pour respecter BDD
    ui->LeModifDateService->setDisplayFormat("yyyy-MM-dd");
    ui->LeModifDatePile->setDisplayFormat("yyyy-MM-dd");




    int num_badge = ui->cBoxModif->currentText().toInt();
    QString id_badge = ui->LeModifIdBadge->text();
    QString dateMiseEnService = ui->LeModifDateService->text();
    QString dateChangerPile = ui->LeModifDatePile->text();

    //modifier formatDate pour affichage FR
    ui->LeModifDateService->setDisplayFormat("dd/MM/yyyy");
    ui->LeModifDatePile->setDisplayFormat("dd/MM/yyyy");


    if(EtatLier==true)
    //requête
    modifier = pBdd->setModBadge(num_badge, id_badge, dateMiseEnService, dateChangerPile);
    if(!modifier){
    //erreur
    QMessageBox::warning(0, tr("Attention : requete impossible"),
     tr("Impossible de modifier ce badge.\nVerifier les champs.\nErreur 002."),
              QMessageBox::Ok);

    }else{
    QMessageBox::information(0, tr("Modifier un Badge a une Personne"),
     tr("Operation reussie.\n"),
              QMessageBox::Ok);
    this->actuAffichage();

    }//if
}

void Configurer::on_cBoxAffecterBadge_activated(int index)
{

    //si non foncitonnement avec index changer par numBadge
    QString id_badge =listeBadge.at(index).id_badge;
    QString num_badge = listeBadge.at(index).numBadge;
    QString dateMiseEnService = listeBadge.at(index).dateMiseEnService;
    QString dateChangePile = listeBadge.at(index).dateChangePile;
    ui->txtInfoBadge->clear();
    ui->txtInfoBadge->textCursor().insertText("Numéro de Badge : "+num_badge+"             Identifiant du badge : "+id_badge+" \nDate de mise en service : "+dateMiseEnService+" \nDate de changement Pile : "+dateChangePile);

}


//////////////////////////////////////////SLOT VUES////////////////////////////////////////////////////////////////////////////////////////
//ONGLET Configurer super / GESTION DES VUES
//Boutons annulés

void Configurer::on_btAnnulerVueAdd_clicked()
{
    //remise à zéro
    ui->lEditNumVueAdd->clear();
    ui->txtVueExiste->clear();
    ui->txtVueLegendeAdd->clear();
    ui->txtVueImageAdd->clear();
}

void Configurer::on_btAnnulerVueMod_clicked()
{
    ui->cBoxVueMod->setCurrentIndex(0);
    ui->lEditNumVueMod->clear();
    ui->txtVueLegendeMod->clear();
    ui->txtVueImageMod->clear();
}

void Configurer::on_btannulerVueSupp_clicked()
{
    ui->cBoxVueSupp->setCurrentIndex(0);
    ui->lEditNumVueSupp->clear();
    ui->txtVueLegendeSupp->clear();
    ui->txtVueImageSupp->clear();
}
//++++++++++
//obtenir vue existante
void Configurer::on_btVueExistante_clicked()
{
    //nettoyer vue
    listeVue.clear();

    //initialisation
    ui->txtVueExiste->clear();

    //récupération des infos
    pBdd->getVueExistant(&listeVue);

    if(!listeVue.empty()){
        for(int i = 0; i < listeVue.count(); i++){
            QString numVue = listeVue.at(i).numVue;
          //  QString legende = listeVue.at(i).legende;
          //  QString image = listeVue.at(i).image;

            //création chaine et affichage
            QString vueExistant = numVue + " ; ";
            ui->txtVueExiste->textCursor().insertText(vueExistant);
        }
    }else{
        ui->txtVueExiste->textCursor().insertText("Aucune Vue.");
    }
}
//++++++++++
//ajouter vue
void Configurer::on_btOkVueAdd_clicked()
{
    //récupération des informations dans les champs
    QString numVue = ui->lEditNumVueAdd->text();
    QString legende = ui->txtVueLegendeAdd->toPlainText();
    QString image = ui->txtVueImageAdd->toPlainText();

    //requête
    bool addVue = pBdd->setVue(numVue, legende, image);

    if(!addVue){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible d'ajouter cette vue.\nVerifier les champs.\nErreur 004."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Ajouter une Vue"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->on_btAnnulerVueAdd_clicked();
        this->actuAffichage();
    }
}
//++++++++++
//SLOT combobox vue mod
void Configurer::on_cBoxVueMod_activated(int index)
{
    QString numVue = listeVue.at(index).numVue;
    QString legende = listeVue.at(index).legende;
    QString image = listeVue.at(index).image;

    //ajout champs
    ui->lEditNumVueMod->clear();
    ui->lEditNumVueMod->insert(numVue);
    ui->txtVueLegendeMod->clear();
    ui->txtVueLegendeMod->textCursor().insertText(legende);
    ui->txtVueImageMod->clear();
    ui->txtVueImageMod->textCursor().insertText(image);

}
//++++++++++
//SLOT bouton ok modifier vue
void Configurer::on_btOkVueMod_clicked()
{
    //récupération des informations dans les champs
    QString legendeActuelle = ui->cBoxVueMod->currentText();
    QString numVue = ui->lEditNumVueMod->text();
    QString legende = ui->txtVueLegendeMod->toPlainText();
    QString image = ui->txtVueImageMod->toPlainText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numVue == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier cette Vue.\nLes champs sont vides.\nErreur 020."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool modVue = pBdd->addModVue(legendeActuelle, numVue, legende, image);

        if(!modVue){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier cette vue.\nVerifier les champs.\nErreur 005."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier une Vue"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerVueMod_clicked();
            this->actuAffichage();
        }
    }
}
//++++++++++
//SLOT combobox supp vue
void Configurer::on_cBoxVueSupp_activated(int index)
{
    QString numVue = listeVue.at(index).numVue;
    QString legende = listeVue.at(index).legende;
    QString image = listeVue.at(index).image;

    //ajout champs
    ui->lEditNumVueSupp->clear();
    ui->lEditNumVueSupp->insert(numVue);
    ui->txtVueLegendeSupp->clear();
    ui->txtVueLegendeSupp->textCursor().insertText(legende);
    ui->txtVueImageSupp->clear();
    ui->txtVueImageSupp->textCursor().insertText(image);
}
//++++++++++
//SLOT bouton ok supprimer vue
void Configurer::on_btOKVueSupp_clicked()
{
    //récupération des informations dans les champs
    QString numVue = ui->lEditNumVueSupp->text();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numVue == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer cette vue.\nLes champs sont vides.\nErreur 021."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool supp = pBdd->setSuppVue(numVue);

        if(!supp){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de supprimer cette vue.\nVerifier les champs.\nErreur 006."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Supprimer une vue"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btannulerVueSupp_clicked();
            this->actuAffichage();
        }
    }
}
///////////////////////////////////////////SLOT LIEUX//////////////////////////////////////////////////////////////////////////////
//onglet GESTION DES LIEUX
//Boutons annulés

void Configurer::on_btAnnulerLieuAdd_clicked()
{
    ui->lEditLieuNumAdd->clear();
    ui->txtLieuExiste->clear();
    ui->txtLieuLegendeAdd->clear();
}

void Configurer::on_btAnnulerLieuMod_clicked()
{
    ui->cBoxLieuMod->setCurrentIndex(0);;
    ui->lEditLieuNumMod->clear();
    ui->txtLieuLegendeMod->clear();
}

void Configurer::on_btAnnulerLieuSupp_clicked()
{
   ui->cBoxLieuSupp->setCurrentIndex(0);
   ui->lEditLieuNumSupp->clear();
   ui->txtLieuLegendeSupp->clear();
}
//++++++++++
//obtenir lieu existant
void Configurer::on_btLieuExistant_clicked()
{
    //nettoyer vue
    listeLieu.clear();

    //initialisation
    ui->txtLieuExiste->clear();

    //récupération des infos
    pBdd->getLieuExistant(&listeLieu);

    if(!listeLieu.empty()){
        for(int i = 0; i < listeLieu.count(); i++){
            QString numLieu = listeLieu.at(i).numLieu;
          //  QString legende = listeVue.at(i).legende;

            //création chaine et affichage
            QString lieuExistant = numLieu + " ; ";
            ui->txtLieuExiste->textCursor().insertText(lieuExistant);
        }
    }else{
        ui->txtLieuExiste->textCursor().insertText("Aucun Lieu.");
    }
}

//++++++++++
//ajouter lieu
void Configurer::on_btOkLieuAdd_clicked()
{
    //récupération des informations dans les champs
    QString numLieu = ui->lEditLieuNumAdd->text();
    QString legende = ui->txtLieuLegendeAdd->toPlainText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numLieu == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible d'ajouter ce Lieu.\nLes champs sont vides.\nErreur 022."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool addLieu = pBdd->setLieu(numLieu, legende);

        if(!addLieu){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible d'ajouter ce Lieu.\nVerifier les champs.\nErreur 007."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Ajouter un Lieu"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerLieuAdd_clicked();
            this->actuAffichage();
        }
    }
}

//++++++++++
//SLOT combobox lieu mod
void Configurer::on_cBoxLieuMod_activated(int index)
{
    QString numLieu = listeLieu.at(index).numLieu;
    QString legende = listeLieu.at(index).legende;

    //ajout champs
    ui->lEditLieuNumMod->clear();
    ui->lEditLieuNumMod->insert(numLieu);
    ui->txtLieuLegendeMod->clear();
    ui->txtLieuLegendeMod->textCursor().insertText(legende);
}

//++++++++++
//SLOT combobox lieu supp
void Configurer::on_cBoxLieuSupp_activated(int index)
{
    QString numLieu = listeLieu.at(index).numLieu;
    QString legende = listeLieu.at(index).legende;

    //ajout champs
    ui->lEditLieuNumSupp->clear();
    ui->lEditLieuNumSupp->insert(numLieu);
    ui->txtLieuLegendeSupp->clear();
    ui->txtLieuLegendeSupp->textCursor().insertText(legende);
}

//++++++++++
//SLOT bouton ok modifier lieu
void Configurer::on_btOkLieuMod_clicked()
{
    //récupération des informations dans les champs
    QString legendeActuelle = ui->cBoxLieuMod->currentText();
    QString numLieu = ui->lEditLieuNumMod->text();
    QString legende = ui->txtLieuLegendeMod->toPlainText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numLieu == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier ce Lieu.\nLes champs sont vides.\nErreur 023."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool modLieu = pBdd->addModLieu(legendeActuelle, numLieu, legende);

        if(!modLieu){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier ce Lieu.\nVerifier les champs.\nErreur 008."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier un Lieu"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerLieuMod_clicked();
            this->actuAffichage();
        }
    }
}

//++++++++++
//SLOT bouton ok supprimer lieu
void Configurer::on_btOkLieuSupp_clicked()
{
    //récupération des informations dans les champs
    QString numLieu = ui->lEditLieuNumSupp->text();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numLieu == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer ce Lieu.\nLes champs sont vides.\nErreur 024."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool supp = pBdd->setSuppLieu(numLieu);

        if(!supp){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de supprimer ce Lieu.\nCe Lieu est probablement lie avec une zone.\nErreur 009."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Supprimer un Lieu"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerLieuSupp_clicked();
            this->actuAffichage();
        }
    }
}
////////////////////////////////////////SLOT ZONES//////////////////////////////////////////////////////////////////////////////
//ONGLET GESTION des ZONES (config Super)
//boutons annulés

void Configurer::on_btAnnulerZoneAdd_clicked()
{
    ui->lEditNumZoneAdd->clear();
    ui->txtZoneExiste->clear();
    ui->cBoxLieuLier->setCurrentIndex(0);
    ui->lEditZoneSensMonterAdd->clear();
    ui->txtZoneLegendeAdd->clear();
}

void Configurer::on_btAnnulerZoneMod_clicked()
{
    ui->cBoxZoneMod->setCurrentIndex(0);
    ui->lEditNumLieuActuel->clear();
    ui->cBoxZoneLieuMod->setCurrentIndex(0);
    ui->lEditZoneSensMonterMod->clear();
    ui->txtZoneLegendeMod->clear();
}

void Configurer::on_btAnnulerZoneSupp_clicked()
{
    ui->cBoxZoneSupp->setCurrentIndex(0);
    ui->txtZoneLieuSupp->clear();
    ui->lEditZoneSensMonterSupp->clear();
    ui->txtZoneLegendeSupp->clear();
}

//obtenir zones existantes
void Configurer::on_btVueExistante_2_clicked()
{
    //nettoyer vue
    listeZone.clear();

    //initialisation
    ui->txtZoneExiste->clear();

    //récupération des infos
    pBdd->getZoneExistant(&listeZone);

    if(!listeZone.empty()){
        for(int i = 0; i < listeZone.count(); i++){
            QString numZone = listeZone.at(i).numZone;

            //création chaine et affichage
            QString zoneExistant = numZone + " ; ";
            ui->txtZoneExiste->textCursor().insertText(zoneExistant);
        }
    }else{
        ui->txtZoneExiste->textCursor().insertText("Aucune Zone.");
    }
}
//ajouter zone
void Configurer::on_btOkZoneAdd_clicked()
{
    //récupération des informations dans les champs
    QString numZone = ui->lEditNumZoneAdd->text();
    QString legendeLieu = ui->cBoxLieuLier->currentText();
    QString sensMonter = ui->lEditZoneSensMonterAdd->text();
    QString legende = ui->txtZoneLegendeAdd->toPlainText();

    if(sensMonter == ""){
        //évite les enregistrements involontaires
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible d'ajouter cette Zone.\nVerifier les champs.\nErreur 013."),
                              QMessageBox::Ok);
    }
    else{
        //obtenir le numéro de la personne
        int numLieu = pBdd->getNumLieu(legendeLieu);

        if(numLieu == -1){  //Problème
            QMessageBox::warning(0, tr("Attention : erreur"),
                         tr("Impossible de trouver ce Lieu.\nVerifier les champs.\nErreur 010."),
                                  QMessageBox::Ok);
        }
        //requête
        bool addZone = pBdd->setZone(numZone, numLieu, sensMonter, legende);

        if(!addZone){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible d'ajouter cette Zone.\nVerifier les champs.\nErreur 011."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Ajouter une Zone"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerZoneAdd_clicked();
            this->actuAffichage();
        }
    }
}
//SLOT combobox zone mod
void Configurer::on_cBoxZoneMod_activated(int index)
{
    QString numLieuActuel = listeZone.at(index).numLieu;
    QString sensMonter = listeZone.at(index).sensMonter;
    QString legende = listeZone.at(index).legende;

    //ajout champs
    ui->lEditNumLieuActuel->clear();
    ui->lEditNumLieuActuel->insert(numLieuActuel);
    ui->lEditZoneSensMonterMod->clear();
    ui->lEditZoneSensMonterMod->insert(sensMonter);
    ui->txtZoneLegendeMod->clear();
    ui->txtZoneLegendeMod->textCursor().insertText(legende);
}
//SLOT bouton ok zone mod
void Configurer::on_btOkZoneMod_clicked()
{
    //récupération des informations dans les champs
    QString numZone = ui->cBoxZoneMod->currentText();
    QString numLieuActuel = ui->lEditNumLieuActuel->text();
    QString numLieu = ui->cBoxZoneLieuMod->currentText();
    QString sensMonter = ui->lEditZoneSensMonterMod->text();
    QString legende = ui->txtZoneLegendeMod->toPlainText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(sensMonter == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier cette Zone.\nLes champs sont vides.\nErreur 025."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool modZone = pBdd->addModZone(numZone, numLieuActuel, numLieu, sensMonter, legende);

        if(!modZone){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier cette Zone.\nVerifier les champs.\nErreur 012."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier une Zone"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerZoneMod_clicked();
            this->actuAffichage();
        }
    }
}
//SLOT combobox zone supp
void Configurer::on_cBoxZoneSupp_activated(int index)
{
    QString numLieu = listeZone.at(index).numLieu;
    QString sensMonter = listeZone.at(index).sensMonter;
    QString legende = listeZone.at(index).legende;

    //ajout champs
    ui->txtZoneLieuSupp->clear();
    ui->txtZoneLieuSupp->textCursor().insertText(numLieu);
    ui->lEditZoneSensMonterSupp->clear();
    ui->lEditZoneSensMonterSupp->insert(sensMonter);
    ui->txtZoneLegendeSupp->clear();
    ui->txtZoneLegendeSupp->textCursor().insertText(legende);
}
//SLOT bouton ok zone supp
void Configurer::on_btOkZoneSupp_clicked()
{
    //récupération des informations dans les champs
    QString numZone = ui->cBoxZoneSupp->currentText();
    QString numLieu = ui->txtZoneLieuSupp->toPlainText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numLieu == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer ce Lieu.\nLes champs sont vides.\nErreur 026."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool supp = pBdd->setSuppZone(numZone);

        if(!supp){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de supprimer cette Zone.\nVerifier les champs.\nErreur 014."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Supprimer une Zone"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerZoneSupp_clicked();
            this->actuAffichage();
        }
    }
}
///////////////////////////////////////////SLOT POSITIONS///////////////////////////////////////////////////////////////////////////////
//ONGLET GESTION POSITIONS

//combobox numéro de vue
void Configurer::on_cBoxPositionNumVue_activated(int index)
{
    QString legende = listeVue.at(index).legende;

    //ajout champs
    ui->txtPositionInfoVue->clear();
    ui->txtPositionInfoVue->textCursor().insertText(legende);
}
//combobox numéro de zone
void Configurer::on_cBoxPositionNumZone_activated(int index)
{
    QString legende = listeZone.at(index).legende;
    QString numLieu = listeZone.at(index).numLieu;

    //ajout champs
    ui->txtPositionInfoZone->clear();
    ui->txtPositionInfoZone->textCursor().insertText(legende);
    ui->lEditPositionNumLieu->clear();
    ui->lEditPositionNumLieu->insert(numLieu);

    //obtenir legende Lieu
    QString legendeLieu = pBdd->getLegendeLieu(numLieu);
    //ajout champs
    ui->txtPositionInfoLieu->clear();
    if(legendeLieu == "" || legendeLieu == "erreur"){
        ui->txtPositionInfoLieu->textCursor().insertText("Aucune légende");
    }else{
        ui->txtPositionInfoLieu->textCursor().insertText(legendeLieu);
    }

    //recherche si position existe déjà
    //récupération info nécessaire dans les champs
    QString numVue = ui->cBoxPositionNumVue->currentText();
    QString numZone = ui->cBoxPositionNumZone->currentText();

    //nettoyage QList
    listeRepresentation.clear();

    bool representation = pBdd->getRepresentation(&listeRepresentation, numVue, numZone);
    if(!representation){
        this->etatPosition = true;
        //pas de position enregistré
        QMessageBox::information(0, tr("Ajouter Coordonnees"),
                     tr("Il n'existe pas encore de position.\nVeuiller remplir les champs.\n"),
                              QMessageBox::Ok);
    }else{
        this->etatPosition = false;
        //position déjà enregistrée
        QMessageBox::information(0, tr("Modifier Coordonnees"),
                     tr("Il existe deja des positions.\nVeuiller remplir les champs pour faire des modifications.\n"),
                              QMessageBox::Ok);
        if(!listeRepresentation.empty()){
            for(int i=0; i < listeRepresentation.count(); i++){
                QString x = listeRepresentation.at(i).x;
                QString y = listeRepresentation.at(i).y;
                QString xA = listeRepresentation.at(i).xA;
                QString yA = listeRepresentation.at(i).yA;
                QString xB = listeRepresentation.at(i).xB;
                QString yB = listeRepresentation.at(i).yB;

                //affichage
                ui->lEditPositionLieuX->clear();
                ui->lEditPositionLieuX->insert(x);
                ui->lEditPositionLieuY->clear();
                ui->lEditPositionLieuY->insert(y);
                ui->lEditPositionZoneXA->clear();
                ui->lEditPositionZoneXA->insert(xA);
                ui->lEditPositionZoneYA->clear();
                ui->lEditPositionZoneYA->insert(yA);
                ui->lEditPositionZoneXB->clear();
                ui->lEditPositionZoneXB->insert(xB);
                ui->lEditPositionZoneYB->clear();
                ui->lEditPositionZoneYB->insert(yB);

            }
        }else{
            //affichage par défaut (0)
            ui->lEditPositionLieuX->clear();
            ui->lEditPositionLieuX->insert(0);
            ui->lEditPositionLieuY->clear();
            ui->lEditPositionLieuY->insert(0);
            ui->lEditPositionZoneXA->clear();
            ui->lEditPositionZoneXA->insert(0);
            ui->lEditPositionZoneYA->clear();
            ui->lEditPositionZoneYA->insert(0);
            ui->lEditPositionZoneXB->clear();
            ui->lEditPositionZoneXB->insert(0);
            ui->lEditPositionZoneYB->clear();
            ui->lEditPositionZoneYB->insert(0);
        }

    }//fin representation
}
//bouton annulé ajouter/modifier position
void Configurer::on_btAnnulerPosition_clicked()
{
    ui->cBoxPositionNumVue->setCurrentIndex(0);
    ui->txtPositionInfoVue->clear();
    ui->cBoxPositionNumZone->setCurrentIndex(0);
    ui->txtPositionInfoZone->clear();
    ui->lEditPositionZoneXA->clear();
    ui->lEditPositionZoneYA->clear();
    ui->lEditPositionZoneXB->clear();
    ui->lEditPositionZoneYB->clear();
    ui->lEditPositionNumLieu->clear();
    ui->txtPositionInfoLieu->clear();
    ui->lEditPositionLieuX->clear();
    ui->lEditPositionLieuY->clear();
    this->etatPosition = -1;
}
//SLOT bouton ok ajouter/modifier position
void Configurer::on_btOkPosition_clicked()
{
    //récupération des informations dans les champs
    QString numVue = ui->cBoxPositionNumVue->currentText();
    QString numZone = ui->cBoxPositionNumZone->currentText();
    QString numLieu = ui->lEditPositionNumLieu->text();
    QString xA = ui->lEditPositionZoneXA->text();
    QString yA = ui->lEditPositionZoneYA->text();
    QString xB = ui->lEditPositionZoneXB->text();
    QString yB = ui->lEditPositionZoneYB->text();
    QString x = ui->lEditPositionLieuX->text();
    QString y = ui->lEditPositionLieuY->text();

    //ajouter position
    if(this->etatPosition == 1){
        //requête
        bool addPosition = pBdd->setPosition(numVue, numZone, numLieu, xA, yA, xB, yB, x, y);

        if(!addPosition){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible d'ajouter ces coordonnees.\nVerifier les champs.\nErreur 015."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Ajouter Coordonnees"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerPosition_clicked();
            this->etatPosition = -1;
            this->actuAffichage();
        }
    //modifier position
    } else if (this->etatPosition == 0){
        //requête
        bool modPosition = pBdd->addModPosition(numVue, numZone, numLieu, xA, yA, xB, yB, x, y);

        if(!modPosition){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier ces coordonnees.\nVerifier les champs.\nErreur 016."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier Coordonnees"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerPosition_clicked();
            this->etatPosition = -1;
            this->actuAffichage();
        }
    }else{
        //ok
        QMessageBox::warning(0, tr("Attention champs vides"),
                     tr("Operation echouee.\nLes champs sont vides.\nErreur 017."),
                              QMessageBox::Ok);
    }
}
//SLOT comboBox supp vue
void Configurer::on_cBoxPositionSuppNumVue_activated(int index)
{
    QString legende = listeVue.at(index).legende;
    QString numVue = listeVue.at(index).numVue;

    //ajout champs
    ui->txtPositionSuppInfoVue->clear();
    ui->txtPositionSuppInfoVue->textCursor().insertText(legende);

    //nettoyer combobox
    ui->cBoxPositionSuppNumZone->clear();

    //nettoyer QList
    listeRepresentation.clear();

    //obtenir les zones liées avec cette vue
    pBdd->getZoneLierVue(&listeRepresentation, numVue);

    if(!listeRepresentation.empty()){
        for(int i = 0; i < listeRepresentation.count(); i++) {
            QString numZone = listeRepresentation.at(i).numZone;

            //ajout du combo
            ui->cBoxPositionSuppNumZone->addItem(numZone);
        }
    }
}
//SLOT comboBox supp Zone
void Configurer::on_cBoxPositionSuppNumZone_activated(int index)
{
    QString legende = listeZone.at(index).legende;
    QString numLieu = listeZone.at(index).numLieu;

    //ajout champs
    ui->txtPositionSuppInfoZone->clear();
    ui->txtPositionSuppInfoZone->textCursor().insertText(legende);
    ui->lEditPositionSuppNumLieu->clear();
    ui->lEditPositionSuppNumLieu->insert(numLieu);

    //obtenir legende Lieu
    QString legendeLieu = pBdd->getLegendeLieu(numLieu);
    //ajout champs
    ui->txtPositionSuppInfoLieu->clear();
    if(legendeLieu == "" || legendeLieu == "erreur"){
        ui->txtPositionSuppInfoLieu->textCursor().insertText("Aucune légende");
    }else{
        ui->txtPositionSuppInfoLieu->textCursor().insertText(legendeLieu);
    }

    //récupération des champs
    QString numVue = ui->cBoxPositionSuppNumVue->currentText();
    QString numZone = ui->cBoxPositionSuppNumZone->currentText();

    listeRepresentation.clear();

    pBdd->getRepresentation(&listeRepresentation, numVue, numZone);

    if(!listeRepresentation.empty()){
        for(int i=0; i < listeRepresentation.count(); i++){
            QString x = listeRepresentation.at(i).x;
            QString y = listeRepresentation.at(i).y;
            QString xA = listeRepresentation.at(i).xA;
            QString yA = listeRepresentation.at(i).yA;
            QString xB = listeRepresentation.at(i).xB;
            QString yB = listeRepresentation.at(i).yB;

            //affichage
            ui->lEditPositionSuppLieuX->clear();
            ui->lEditPositionSuppLieuX->insert(x);
            ui->lEditPositionSuppLieuY->clear();
            ui->lEditPositionSuppLieuY->insert(y);
            ui->lEditPositionSuppZoneXA->clear();
            ui->lEditPositionSuppZoneXA->insert(xA);
            ui->lEditPositionSuppZoneYA->clear();
            ui->lEditPositionSuppZoneYA->insert(yA);
            ui->lEditPositionSuppZoneXB->clear();
            ui->lEditPositionSuppZoneXB->insert(xB);
            ui->lEditPositionSuppZoneYB->clear();
            ui->lEditPositionSuppZoneYB->insert(yB);
        }
    }
}
//SLOT bouton annulé supp positions
void Configurer::on_btAnnulerPositionSupp_clicked()
{
    ui->cBoxPositionSuppNumVue->setCurrentIndex(0);
    ui->txtPositionSuppInfoVue->clear();
    ui->cBoxPositionSuppNumZone->clear();
    ui->txtPositionSuppInfoZone->clear();
    ui->lEditPositionSuppZoneXA->clear();
    ui->lEditPositionSuppZoneYA->clear();
    ui->lEditPositionSuppZoneXB->clear();
    ui->lEditPositionSuppZoneYB->clear();
    ui->lEditPositionSuppNumLieu->clear();
    ui->txtPositionSuppInfoLieu->clear();
    ui->lEditPositionSuppLieuX->clear();
    ui->lEditPositionSuppLieuY->clear();
}
//SLOT bouton ok supp positions
void Configurer::on_btOkPositionSupp_clicked()
{
    //récupération des informations dans les champs
    QString numVue = ui->cBoxPositionSuppNumVue->currentText();
    QString numZone = ui->cBoxPositionSuppNumZone->currentText();

    //évite d'avoir appuyer sur le bouton par mégarde
    if(numZone == ""){
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer ces coordonnees.\nLes champs sont vides.\nErreur 027."),
                              QMessageBox::Ok);
    }
    else{
        //requête
        bool supp = pBdd->setSuppPosition(numVue, numZone);

        if(!supp){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de supprimer ces coordonnees.\nVerifier les champs.\nErreur 018."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Supprimer Coordonnees"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            this->on_btAnnulerPositionSupp_clicked();
            this->actuAffichage();
        }
    }
}
////////////////////////////////////////////SLOTS TIMERS/////////////////////////////////////////////////////////////////////////////
//ONGLET GESTION DES TIMERS

//SLOT bouton ok régler timer
void Configurer::on_btOkTimer_clicked()
{
    int clic = QMessageBox::warning(0, tr("Attention : modification des durees"),
             tr("Etes vous sur de vouloir modifier les durees des Timers ?\n"),
                      QMessageBox::Ok ,QMessageBox::Cancel);

    if(clic == QMessageBox::Ok){
        //récupération des champs
        QString tempoMouv = ui->lEditTimerMouv->text();
        QString tempoRecep = ui->lEditTimerRecep->text();

        if(tempoMouv == "" || tempoRecep == "" || tempoMouv == "0" || tempoRecep == "0"){
            QMessageBox::warning(0, tr("Attention : champs vides"),
                         tr("Vous ne pouvez pas laisser des champs vides.\nErreur 028."),
                                  QMessageBox::Ok);
        }else{
            //Conversion en millisecondes
            float f_tempoMouv = tempoMouv.toFloat() * 1000;
            float f_tempoRecep = tempoRecep.toFloat() * 1000 ;

            //requête
            bool regler = pBdd->setTempo(QString::number(f_tempoMouv), QString::number(f_tempoRecep));
            if(!regler){
                //erreur
                QMessageBox::warning(0, tr("Attention : requete impossible"),
                                     tr("Impossible de modifier les durees des Timers.\nVerifier les champs.\nErreur 019."),
                                      QMessageBox::Ok);
            }else{
                //ok
                QMessageBox::information(0, tr("Modifier durees Timers"),
                             tr("Operation reussie.\n"),
                                      QMessageBox::Ok);
                this->actuAffichage();
            }//si requete bien passée
        }//si champs vide
   }//si modif ok
}











//////////////////////////ONGLET GESTION LECTEURS ----- MODULE AJOUTE PAR T.DRONNE --- MODIFIE PAR CHAMART 2015///////////////////////////////////////////////

//SLOT Affichage des informations lors de la sélection d'un lecteur en vue de la suppression
void Configurer::on_cBoxLectSupp_activated(int index)
{
    QString ipLecteur = listeLecteur.at(index).ipLecteur;
    QString lieuLecteur = listeLecteur.at(index).numLieuLecteur;
    QString numLecteur = listeLecteur.at(index).numLecteur;

    //ajout dans la box d'info du badge
    ui->txtInfoLectDel->clear();
    ui->txtInfoLectDel->textCursor().insertText("Lecteur nÂ° : "+numLecteur+"\nAdresse IP : " + ipLecteur + "\nNuméro du lieu associé : "+ lieuLecteur +"\n\n");
}


//SLOT lors de sélection du lecteur Ã  modifier
void Configurer::on_cBoxNumLecteursModif_activated(int index)
{
    QString ipLecteur = listeLecteur.at(index).ipLecteur;
    int lieuLecteur = listeLecteur.at(index).numLieuLecteur.toInt();

    //ajout dans le textEdit de l'adresse ip actuelle du lecteur
    ui->lEditIPLecteur_Mod->clear();
    ui->lEditIPLecteur_Mod->setText(ipLecteur);

    //placement de la combobox du lieu pour le lecteur selectionné
    ui->cBoxLieuxForLecteur_Modif->setCurrentIndex(lieuLecteur - 1);

}



//LES BOUTONS ANNULER...
void Configurer::on_btAnnulerLectAdd_clicked() //Annuler l'ajout
{
    //On vide et on réinitialise les champs.
    ui->lEditIPLecteur->clear();
    ui->cBoxLieuxForLecteur->setCurrentIndex(0);
}

void Configurer::on_btAnnulerLectModif_clicked()
{
    //On vide et on réinitialise les champs.
    ui->cBoxNumLecteursModif->setCurrentIndex(0);
    ui->lEditIPLecteur_Mod->clear();
    ui->cBoxLieuxForLecteur_Modif->setCurrentIndex(0);

}

void Configurer::on_btAnnulerLectSupp_clicked()
{
    ui->cBoxLectSupp->setCurrentIndex(0);
    ui->txtInfoLectDel->clear();
}


//LES VALIDATIONS !!!

//SLOT Validation ajout lecteur
void Configurer::on_btOkAddLect_clicked()
{

    //récupération
    int i_numLecteur = listeLecteur.last().numLecteur.toInt() + 1;
    QString numLecteur = QString::number(i_numLecteur);    /*ui->lEditNumLecteur->text();*/
    QString numLieuLecteur = ui->cBoxLieuxForLecteur->currentText();
    QString ipLecteur = ui->lEditIPLecteur->text();

    //requÃªte
    bool add = pBdd->addLecteur(numLecteur, numLieuLecteur, ipLecteur);


    if(!add){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible d'ajouter ce lecteur.\nVerifier les champs.\nErreur 006."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Ajouter un lecteur"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->on_btAnnulerLectAdd_clicked();
        this->actuAffichage();

        //indiquer qu'il faut parametrer le lecteur
        ui->lbParametrerLecteur->setVisible(true);
        ui->lbParametrerLecteur_2->setVisible(true);

    }

}


//SLOT validation modifier lecteur
void Configurer::on_btOkLectModif_clicked()
{

    //récupération
    QString numLecteur = ui->cBoxNumLecteursModif->currentText();
    QString numLieuLecteur = ui->cBoxLieuxForLecteur_Modif->currentText();
    QString ipLecteur = ui->lEditIPLecteur_Mod->text();

    //requÃªte
    bool mod = pBdd->setLecteur(numLecteur, numLieuLecteur, ipLecteur);


    if(!mod){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier ce lecteur.\nVerifier les champs.\nErreur 006."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Modifier un lecteur"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->on_btAnnulerLectModif_clicked();
        this->actuAffichage();

    }

}



//SLOT validation supprimer lecteur
void Configurer::on_btOkLectSupp_clicked()
{
    //récupération des informations dans les champs
    QString numLecteur = ui->cBoxLectSupp->currentText();

    //requÃªte
    bool supp = pBdd->setSuppLecteur(numLecteur);

    if(!supp){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer ce lecteur.\nVerifier les champs.\nErreur 006."),
                              QMessageBox::Ok);
    }else{
        //ok
        QMessageBox::information(0, tr("Supprimer un lecteur"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        this->on_btAnnulerLectSupp_clicked();
        this->actuAffichage();

    }
}



//////////////////////////ONGLET GESTION PERSONNES ----- MODULE AJOUTE PAR DURAND  ----------   MODIFIE PAR CHAMART 2015 /////////////////////////////////////

/* ----------------------------------------------------------------------------------- */

//SLOT Affichage des informations lors de la sélection d'une personne en vue de la suppression

    void Configurer::on_cBoxPersSupp_activated(int index)
    {
        QString portable = listePersonne.at(index).portable;
        QString email = listePersonne.at(index).email;
        QString nom = listePersonne.at(index).nom;
        QString prenom = listePersonne.at(index).prenom;
        QString societe = listePersonne.at(index).societe;
        QString dateFin = listePersonne.at(index).dateFin;
        QDate fin=fin.fromString(dateFin,"yyyy-MM-dd");

        //ajout dans la box d'info de le personne
        ui->txtInfoPersDel->clear();
        ui->txtInfoPersDel->textCursor().insertText("Nom : " + nom + "\nPrenom : " + prenom + "\nPortable : "+portable+"\nE-mail : "+email+"\nSociété : "+ societe + "\nDate de fin : " + dateFin);
    }

/* ----------------------------------------------------------------------------------- */

//SLOT lors de la sélection de personnes Ã  modifier

    void Configurer::on_cBoxPersModif_activated(int index)
    {
        QString nom = listePersonne.at(index).nom;
        QString prenom = listePersonne.at(index).prenom;
        QString societe = listePersonne.at(index).societe;
        QString dateDebut = listePersonne.at(index).dateDebut;
        QString dateFin = listePersonne.at(index).dateFin;
        QString portable = listePersonne.at(index).portable;
        QString email = listePersonne.at(index).email;

        //convertir les string en QDate pour affichage
        QDate debut;
        debut=debut.fromString(dateDebut,"yyyy-MM-dd");
        QDate fin;
        fin=fin.fromString(dateFin,"yyyy-MM-dd");

        //Nettoyage préliminaire
        ui->ModifierNomPersonne->clear();
        ui->ModifierPrenomPersonne->clear();
        ui->ModifierSociete->clear();
        ui->ModifierDateDebut->clear();
        ui->ModifierDateFin->clear();
        ui->ModifierPortable->clear();
        ui->ModifierEmail->clear();

        //Modifications
        ui->ModifierNomPersonne->setText(nom);
        ui->ModifierPrenomPersonne->setText(prenom);
        ui->ModifierSociete->setText(societe);
        ui->ModifierDateDebut->setDate(debut);
        ui->ModifierDateFin->setDate(fin);
        ui->ModifierPortable->setText(portable);
        ui->ModifierEmail->setText(email);

    }

/* --------------------------------------LES BOUTONS ANNULER--------------------------------------------- */

// Annuler modification de personne

    void Configurer::on_btAnnulerPersMod_clicked()
    {
        ui->cBoxPersModif->setCurrentIndex(0);
        ui->ModifierNomPersonne->clear();
        ui->ModifierPrenomPersonne->clear();
        ui->ModifierSociete->clear();
        ui->ModifierDateDebut->clear();
        ui->ModifierDateFin->clear();
    }

/* ----------------------------------------------------------------------------------- */

// Annuler suppression de personne

    void Configurer::on_btAnnulerPersSupp_clicked()
    {
        ui->txtInfoPersDel->clear();
        ui->cBoxPersSupp->setCurrentIndex(0);
    }

/* ----------------------------------------------------------------------------------- */

// Annuler ajout de personne

    void Configurer::on_btAnnulerPersAdd_clicked()
    {
        ui->nom->clear();
        ui->prenom->clear();
        ui->societe->clear();
        ui->dateDebut->clear();
        ui->dateFin->clear();
    }

/* ------------------------------------------LES VALIDATIONS----------------------------------------- */

// Validation ajout de personne

    void Configurer::on_btOkPersAdd_clicked()
    {

            //modifier formatDate pour respecter BDD
            ui->dateDebut->setDisplayFormat("yyyy-MM-dd");
            ui->dateFin->setDisplayFormat("yyyy-MM-dd");

            //Récupération
            int num_pers=pBdd->getNouveauNumPersonne();
            QString nom = ui->nom->text();
            QString prenom = ui->prenom->text();
            QString portable = ui->portable->text();
            QString email = ui->email->text();
            QString societe = ui->societe->text();
            QString dateDebut = ui->dateDebut->text();
            QString dateFin = ui->dateFin->text();

            //modifier formatDate pour affichage FR
            ui->dateDebut->setDisplayFormat("dd/MM/yyyy");
            ui->dateFin->setDisplayFormat("dd/MM/yyyy");


            //requÃªte
            bool add = pBdd->addPersonne(num_pers, nom, prenom, email, portable, societe, dateDebut, dateFin);

            if(!add){
                //erreur
                QMessageBox::warning(0, tr("Attention : requete impossible"),
                                     tr("Impossible d'ajouter cette personne.\nVerifier les champs.\nErreur 006."),
                                      QMessageBox::Ok);
            }else{
                //ok
                QMessageBox::information(0, tr("Ajouter une personne"),
                             tr("Operation reussie.\n"),
                                      QMessageBox::Ok);
                this->on_btAnnulerPersAdd_clicked();
                this->actuAffichage();

            }



    }

/* ----------------------------------------------------------------------------------- */

    //SLOT validation modifier personne

    void Configurer::on_btOkPersMod_clicked()
    {

        //modifier formatDate pour respecter BDD
        ui->ModifierDateFin->setDisplayFormat("yyyy-MM-dd");
        ui->ModifierDateDebut->setDisplayFormat("yyyy-MM-dd");

        //récupération
        QString selecteDnom = ui->cBoxPersModif->currentText();
        QString nom = ui->ModifierNomPersonne->text();
        QString prenom = ui->ModifierPrenomPersonne->text();
        QString societe = ui->ModifierSociete->text();
        QString dateDebut = ui->ModifierDateDebut->text();
        QString dateFin = ui->ModifierDateFin->text();
        QString email = ui->ModifierEmail->text();
        QString portable = ui->ModifierPortable->text();

        //modifier formatDate pour affichage FR
        ui->ModifierDateFin->setDisplayFormat("dd/MM/yyyy");
        ui->ModifierDateDebut->setDisplayFormat("dd/MM/yyyy");


        //requÃªte
        bool mod = pBdd->setModPersonne(selecteDnom, nom, prenom, email, portable, societe, dateDebut, dateFin);


        if(!mod){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier cette personne.\nVerifier les champs.\nErreur 006."),
                                  QMessageBox::Ok);
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier une personne"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);
            //this->on_btOkPersMod_clicked();
            this->on_btAnnulerPersMod_clicked();
            this->actuAffichage();

        }


    }


/* ----------------------------------------------------------------------------------- */

// Supprimer Personne

    void Configurer::on_btOkPersSupp_clicked()
    {
        {
            //récupération des informations dans les champs
            QString nom = ui->cBoxPersSupp->currentText();

            //requÃªte
            bool supp = pBdd->setSuppPersonne(nom);

            if(!supp){
                //erreur
                QMessageBox::warning(0, tr("Attention : requete impossible"),
                                     tr("Impossible de supprimer cette personne.\nVerifier les champs.\nErreur 006."),
                                      QMessageBox::Ok);
            }else{
                //ok
                QMessageBox::information(0, tr("Supprimer une personne"),
                             tr("Operation reussie.\n"),
                                      QMessageBox::Ok);
                this->on_btAnnulerPersSupp_clicked();
                this->actuAffichage();

            }
        }
    }




/* ----------------------------------------------------------------------------------- */
////////////////////////////////SLOT LOGIN   ---   AJOUTE PAR CHAMART 2015   /////////////////////////////////////////////////////////////////////////////////////////
////
/// SLOT APPUI BOUTON VERIFICATION
///
void Configurer::on_pbVerificationLogin_clicked()
{
    QString login,password;

    //recuperer le login saisi
    login=ui->lnLogin->text();

    //Recuperer le mdp saisie
    password=ui->lnPassword->text();

    //recupérer liste des logins
    QList<QString> listeLogin = pBdd->getLogin();

    //recupérer liste des password
    QList<QString> listePassword = pBdd->getPassword();

    int i;
    bool res=false;

    //parcourir liste de login
    for(i=0;i<listeLogin.count();i++)
    {
        //si la liste login comporte le login au même index que la liste password comporte le password
        if(listeLogin.at(i).contains(login) && listePassword.at(i).contains(password))
        {
            //ok
            res =true;
        }
    }

    //si ok
    if(res==true)
    {
        //enregistrer le login actuel pour modifications
        pBdd->selectedLogin=login;

        //nettoyer la lnEdit mot de passe
        ui->lnPassword->clear();

        //selectionner le login pour le modifier si souhaité
        ui->lnLogin->selectAll();

        //rendre invisible ce bouton
        ui->pbVerificationLogin->setVisible(false);

        //rendre visible ce bouton
        ui->pbConfirmerModifLogin->setVisible(true);

        //rendre visible le texte saisi pour verifier erreur
        ui->lnPassword->setEchoMode(QLineEdit::Normal);

        //affecte un nouveau texte
        ui->lbLogin->setText("Nouvel identifiant :");
        ui->lbPassword->setText("Nouveau mot de passe :");
    }else
    {
        //erreur
        QMessageBox::warning(0, tr("Modification des logins"),
                             tr("L'identifiant ou le mot de passe saisi est incorrect"),
                              QMessageBox::Ok);
    }
}

////
/// SLOT APPUI BOUTON CONFIRMER MODIFIFCATION DE LOGIN
///
void Configurer::on_pbConfirmerModifLogin_clicked()
{
    QString login,password;

    //recuperer le login saisi
    login= ui->lnLogin->text();

    //Recuperer le mdp saisie
    password=ui->lnPassword->text();

    //si login différent que actuel
    if(login!=pBdd->selectedLogin)
    {
        //requête
        bool res = pBdd->setLogin(login,pBdd->selectedLogin);

        if(!res){
            //erreur
            QMessageBox::warning(0, tr("Attention : requete impossible"),
                                 tr("Impossible de modifier le login."),
                                  QMessageBox::Ok);
            //vider lnEdit pour nouvelle saisie
            ui->lnLogin->clear();
            ui->lnPassword->clear();
        }else{
            //ok
            QMessageBox::information(0, tr("Modifier le login"),
                         tr("Operation reussie.\n"),
                                  QMessageBox::Ok);

            //vider lnEdit pour nouvelle saisie
            ui->lnLogin->clear();
            ui->lnPassword->clear();
        }
    }

    //requête
    bool res = pBdd->setPassword(password,pBdd->selectedLogin);

    if(!res){
        //erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier le password"),
                              QMessageBox::Ok);
        //vider lnEdit pour nouvelle saisie
        ui->lnLogin->clear();
        ui->lnPassword->clear();
    }else{
        //ok
        QMessageBox::information(0, tr("Modifier le password"),
                     tr("Operation reussie.\n"),
                              QMessageBox::Ok);
        //vider lnEdit pour nouvelle saisie
        ui->lnLogin->clear();
        ui->lnPassword->clear();

        //init affichage pour revenir au départ
        actuAffichage();

    }
}

////
/// SLOT APPUI BOUTON ENTREE lnPASSWORD
///
void Configurer::on_lnPassword_returnPressed()
{
    //si le text correspond Ã  la vérifiaction des logins
    if(ui->lbLogin->text()=="Identifiant actuel :")
    {
        //simuler appui bouton verifier
        on_pbVerificationLogin_clicked();
    }else
    {
        //si non, simuler appui bouton confirmer
        on_pbConfirmerModifLogin_clicked();
    }
}

////
/// SLOT ACTUALISATION DATE/HEURE
///
void Configurer::sltActualiserDate()
{
    //Récuperation de la date actuelle
    QString heureActuelle = QTime::currentTime().toString();
    QString dateActuelle = QDate::currentDate().toString();

    //création de la chaine finale
    QString date("Nous sommes le "+dateActuelle+" et il est "+heureActuelle+".");

    //Affectation de la chaine finale dans un label
    ui->lbDate->setText(date);
}


/* ----------------------------------------------------------------------------------- */



/* ----------------------------------------------------------------------------------- */





                    ////////////////////////////////////
                    /***            CAMERA          ***/
                    ////////////////////////////////////




///            SLOT comboBox lors de la sélection de caméra à modifier            ///
void Configurer::on_cb_ModifCam_activated(int index)
{
    QString Adresse_IP = listeCamera.at(index).Adresse_IP;
    QString URL = listeCamera.at(index).URL;
    QString Port_TCP = listeCamera.at(index).Port_TCP;
    QString Type_Camera = listeCamera.at(index).Type_Camera;
    QString Num_Camera = listeCamera.at(index).Num_Camera;
    QString Num_Lieu = listeCamera.at(index).Num_Lieu;
    QString Num_Vue = listeCamera.at(index).Num_Vue;

    // Nettoyage préliminaire
    ui->leModifAdressIPCam->clear();
    ui->leModifURLCam->clear();
    ui->leModifPortCam->clear();
    ui->leModifTypeCam->clear();
    //ui->leModifNumCam->clear();
    ui->leModifNumLieuCam->clear();
    ui->leModifNumVueCam->clear();

    // Modifications
    ui->leModifAdressIPCam->setText(Adresse_IP);
    ui->leModifURLCam->setText(URL);
    ui->leModifPortCam->setText(Port_TCP);
    ui->leModifTypeCam->setText(Type_Camera);
    //ui->leModifNumCam->setText(Num_Camera);
    ui->leModifNumLieuCam->setText(Num_Lieu);
    ui->leModifNumVueCam->setText(Num_Vue);
}

///            SLOT comboBox Affichage des infos lors de la sélection d'une caméra en vue de la supprimer            ///
void Configurer::on_cb_SupprCam_activated(int index)
{
    QString Adresse_IP = listeCamera.at(index).Adresse_IP;
    QString URL = listeCamera.at(index).URL;
    QString Port_TCP = listeCamera.at(index).Port_TCP;
    QString Type_Camera = listeCamera.at(index).Type_Camera;
    QString Num_Camera = listeCamera.at(index).Num_Camera;
    QString Num_Lieu = listeCamera.at(index).Num_Lieu;
    QString Num_Vue = listeCamera.at(index).Num_Vue;

    // Ajout dans la box d'info de la camera
    ui->teInfoCam->clear();
    ui->teInfoCam->textCursor().insertText("Adresse IP : " + Adresse_IP + "\nURL : " + URL + "\nPort TCP : " + Port_TCP + "\nType Camera : " + Type_Camera + "\nNumero Camera : " + Num_Camera + "\nNumero Lieu : " + Num_Lieu + "\nNumero Vue : " + Num_Vue);
}






///            LES BOUTONS ANNULER            ///

// Annuler ajout de camera
void Configurer::on_pbAnnulerCam_clicked()
{
    ui->leAdressIPCam->clear();
    ui->leURLCam->clear();
    ui->lePortCam->clear();
    ui->leTypeCam->clear();
    //ui->leNumCam->clear();
    ui->leNumLieuCam->clear();
    ui->leNumVueCam->clear();
}



// Annuler modification de camera
void Configurer::on_pbModifCamAnnuler_clicked()
{
    ui->cb_ModifCam->setCurrentIndex(0);
    ui->leModifAdressIPCam->clear();
    ui->leModifURLCam->clear();
    ui->leModifPortCam->clear();
    ui->leModifTypeCam->clear();
    //ui->leModifNumCam->clear();
    ui->leModifNumLieuCam->clear();
    ui->leModifNumVueCam->clear();
}



// Annuler suppression de camera
void Configurer::on_pbAnnulerSuppr_clicked()
{
    ui->teInfoCam->clear();
    ui->cb_SupprCam->setCurrentIndex(0);
}





///            LES BOUTONS CONFIRMER           ///

// Confirmation ajouter camera
void Configurer::on_pbConfirmerCam_clicked()
{
    // Recuperation
    int Num_Camera = pBdd->getNouveauNumCamera();
    QString Adresse_IP = ui->leAdressIPCam->text();
    QString URL = ui->leURLCam->text();
    QString Port_TCP = ui->lePortCam->text();
    QString Type_Camera = ui->leTypeCam->text();
    //QString Num_Camera = ui->leNumCam->text();
    QString Num_Lieu = ui->leNumLieuCam->text();
    QString Num_Vue = ui->leNumVueCam->text();

    // Requete
    bool add = pBdd->addCamera(Adresse_IP, URL, Port_TCP, Type_Camera, Num_Camera, Num_Lieu, Num_Vue);

    if(!add) {
        // Erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible d'ajouter cette camera.\nVerifier les champs.\nErreur 006"),
                             QMessageBox::Ok);
    }
    else {
        // OK
        QMessageBox::information(0, tr("Ajouter une camera"),
                                 tr("Operation reussie.\n"),
                                 QMessageBox::Ok);

        this->on_pbAnnulerCam_clicked();
        this->actuAffichage();
    }
}



// Confirmation modifier camera
void Configurer::on_pbModifCamConfirmer_clicked()
{
    // Recuperation
    QString selectedNum = ui->cb_ModifCam->currentText();
    QString Adresse_IP = ui->leModifAdressIPCam->text();
    QString URL = ui->leModifURLCam->text();
    QString Port_TCP = ui->leModifPortCam->text();
    QString Type_Camera = ui->leModifTypeCam->text();
    QString Num_Lieu = ui->leModifNumLieuCam->text();
    QString Num_Vue = ui->leModifNumVueCam->text();

    // Requete
    bool modif = pBdd->setModCamera(Adresse_IP, URL, Port_TCP, Type_Camera, selectedNum, Num_Lieu, Num_Vue);

    if(!modif) {
        // Erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de modifier cette camera.\nVerifier les champs.\nErreur 006"),
                             QMessageBox::Ok);
    }
    else {
        // OK
        QMessageBox::information(0, tr("Modifier une camera"),
                                 tr("Operation reussie.\n"),
                                 QMessageBox::Ok);

        this->on_pbModifCamAnnuler_clicked();
        this->actuAffichage();
    }
}



// Confirmation supprimer camera
void Configurer::on_pbConfirmerSuppr_clicked()
{
    // Recuperation des informations dans les champs
    QString camera = ui->cb_SupprCam->currentText();

    // Requete
    bool suppr = pBdd->setSuppCamera(camera);

    if(!suppr) {
        // Erreur
        QMessageBox::warning(0, tr("Attention : requete impossible"),
                             tr("Impossible de supprimer cette camera.\nVerifier les champs.\nErreur 006"),
                             QMessageBox::Ok);
    }
    else {
        // OK
        QMessageBox::information(0, tr("Supprimer une camera"),
                                 tr("Operation reussie.\n"),
                                 QMessageBox::Ok);

        this->on_pbAnnulerSuppr_clicked();
        this->actuAffichage();
    }
}
