//////////////////////////////////////////////////////////////////////////////////////////////////////
#include <QDateTime>
#include <QDebug>
#include "bdd.h"
#include <QSettings>


//////////////////////
/*** CONSTRUCTEUR ***/
Bdd::Bdd(QObject *parent) :
    QObject(parent)
{
    pDynamique = new Dynamique; //allocation pointeur sur classe Dynamique

    //    accès BDD
    //    v3
    /***********************************DUPLAN********************************************/
    //creer objet avec paramètre chemin d'access du fichier et format de lecture
    QSettings settings("SuPer.ini", QSettings::IniFormat);
    //prend valeur 1, si non trouver prend valeur 2
    QString ip = settings.value("Adresse/adresse", "localhost").toString();
    QString ndc = settings.value("Identifiants/NomDeCompte", "super2015").toString();
    QString mdp = settings.value("Identifiants/MotDePasse", "super2015").toString();
    QString NomBDD = settings.value("Identifiants/NomBase", "bdd_super2015").toString();

    database = QSqlDatabase::addDatabase("QMYSQL");
    database.setHostName(ip);
    database.setDatabaseName(NomBDD);
    database.setUserName(ndc);//QBytesArray contenant valeur décrypté
    database.setPassword(mdp);//QBytesArray contenant valeur décrypté
    /**************************************************************************************/

    bool ok = database.open();
    if (!ok){
        qDebug() << database.lastError();
        QMessageBox::critical(0, tr("Impossible d'ouvrir la BDD."),
                 tr("Impossible d'etablir une connexion avec la Base de Donnees.\n"),
                          QMessageBox::Cancel);

    }
    query = new QSqlQuery;

    accesBdd=true;
}
/////////////////////
/*** DESTRUCTEUR ***/
Bdd::~Bdd(){
    delete pDynamique;
    delete query;
    database.close();
}

/////////////////////
void Bdd::getTempo(int *tempoM, int *tempoR, int *tempoL){

    //requête
    requete = "SELECT tempoM, tempoR, tempoL ";
    requete += "FROM super ";
    requete += "LIMIT 1";
    query->prepare(requete);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL obtenir tempo" << endl;
    }

    //réponse requête
    query->next();
    *tempoM = query->value(0).toInt();
    *tempoR = query->value(1).toInt();
    *tempoL = query->value(2).toInt();
}



/////////////////////
void Bdd::setLog(int typeLog, int numBadge){

    //obtenir date
    QString dateLog = QDateTime::currentDateTime().toString();

    //legende
    QString legende;

    //gestion le type de log
    switch (typeLog){
        case 1 :    //nouveau badge
            legende = "Nouveau badge";

            //requête
            requete = "INSERT INTO log (num_badge, dateLog,legende) VALUES (:numBadge, :dateLog,:legende)";
            query->prepare(requete);
            query->bindValue(":numBadge", numBadge);
            query->bindValue(":dateLog", dateLog);
            query->bindValue(":legende", legende);
            if(!query->exec())
                qDebug() << "Erreur requete SQL ajout Log" << endl;
            break;

        case 2 :    //badge perte réception
            legende = "Perte de Reception Badge";
            numBadge++; //dû au numéro de badge venant de timerRec

            //requête
            requete = "INSERT INTO log (num_badge, dateLog,legende) VALUES (:numBadge, :dateLog,:legende)";
            query->prepare(requete);
            query->bindValue(":numBadge", numBadge);
            query->bindValue(":dateLog", dateLog);
            query->bindValue(":legende", legende);
            if(!query->exec())
                qDebug() << "Erreur requete SQL ajout Log" << endl;
            break;

        case 3 :    //badge alarme-non mouvement
            legende = "Alarme non-Mouvement Badge";
            //requête
            requete = "INSERT INTO log (num_badge, dateLog,legende) VALUES (:numBadge, :dateLog,:legende)";
            query->prepare(requete);
            query->bindValue(":numBadge", numBadge);
            query->bindValue(":dateLog", dateLog);
            query->bindValue(":legende", legende);
            if(!query->exec())
                qDebug() << "Erreur requete SQL ajout Log cas 3" << endl;
            break;

        default :   //pas de typeLog ? cas impossible
            legende = "Pas d'information";
            //requête
            requete = "INSERT INTO log (num_badge, dateLog,legende) VALUES (:numBadge, :dateLog,:legende)";
            query->prepare(requete);
            query->bindValue(":numBadge", numBadge);
            query->bindValue(":dateLog", dateLog);
            query->bindValue(":legende", legende);
            if(!query->exec())
                qDebug() << "Erreur requete SQL ajout Log" << endl;
            break;
    }
}


///////////////////////////////////////////
void Bdd::setBadgeActif(int numBadge)
{
    //avec le numéro de badge, mettre qu'il n'est pas perdu
    requete = "UPDATE badge ";
    requete += "SET estActif=1 ";
    requete += "WHERE num_badge=:numBadge";
    query->prepare(requete);
    query->bindValue(":numBadge", numBadge);
    if (!query->exec())
        qDebug() << "Erreur requete SQL badge actif (perte badge)" << endl;

}
///////////////////////////////////////////////
void Bdd::setBadgePerdu(int numBadge)
{
    //avec le numéro de badge, mettre qu'il est perdu !
    requete = "UPDATE badge ";
    requete += "SET estActif=0 ";
    requete += "WHERE num_badge=:numBadge";
    query->prepare(requete);
    query->bindValue(":numBadge", numBadge);
    if (!query->exec())
        qDebug() << "Erreur requete SQL badge actif (perte badge)" << endl;

}
////////////////////////////////////////////////
/// CHAMART
bool Bdd::getNomPersonne(int numBadge, QString *nom){
    requete = "SELECT personne.nom ";
    requete += "FROM personne ";
    requete += "NATURAL JOIN badge ";
    requete += "WHERE badge.num_badge=:numBadge";
    query->prepare(requete);
    query->bindValue(":numBadge", numBadge);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL nom personne/badge" << endl;
         return false;
    }
    query->next();

    *nom=query->value(0).toString();

    return true;

}

////////////////////////////////////////////////
bool Bdd::badgeExistant(QList <T_Badge> *listeBadge){
    //obtenir les badges actifs
    requete = "SELECT num_badge ";
    requete += "FROM badge ";
    requete += "WHERE estActif = 1";
    query->prepare(requete);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL badge existant" << endl;
         return false;
    }

    //allocation pointeur
    this->pBadge = new T_Badge;

    //réponse requête
    while(query->next()){
        int num_badge = query->value(0).toInt();

        //ajout sur liste
        this->pBadge->numBadge = num_badge;
        listeBadge->append(*pBadge);
    }

    delete this->pBadge;
    return true;

}

//////////////////////////////////////////////
bool Bdd::getPointsZone(int vue, int zone, T_Point *pointA, T_Point *pointB)
{
    //avec la zone et la vue, obtenir les points de la droite
    requete = "SELECT xA, yA, xB, yB ";
    requete += "FROM representationLieuSurVue ";
    requete += "WHERE num_vue =:vue AND num_zone =:zone";
    query->prepare(requete);
    query->bindValue(":vue", vue);
    query->bindValue(":zone", zone);
    if (!query->exec()) return false;
    if (!query->size()) return false;
    query->next();
    pointA->x=query->value(0).toInt();
    pointA->y=query->value(1).toInt();
    pointB->x=query->value(2).toInt();
    pointB->y=query->value(3).toInt();
    return true;
}

////////////////////
// co/déco lecteur
void Bdd::setEtatLect(int numLecteur, bool etat){

    //avec le numéro de lecteur et son état le mettre à jour dans la BDD
    requete = "UPDATE lecteur ";
    requete += "SET estConnecte=:etat ";
    requete += "WHERE num_lecteur=:numLecteur";
    query->prepare(requete);
    query->bindValue(":etat", etat);
    query->bindValue(":num_lecteur", numLecteur);
    if (!query->exec())
        qDebug() << "Erreur requete SQL co/déco lecteur" << endl;

}
/*--------------------------------------*
 * Méthode pour obtenir le lieu d'un    *
 *  lecteur                             *
 *--------------------------------------*/
int Bdd::getLieuFctLecteur(int numLecteur, QString &legende)
{
    //preparation de la requete
    requete = "SELECT lecteur.num_lieu, lieu.legende ";
    requete += "FROM lecteur, lieu ";
    requete += "WHERE lecteur.num_lecteur=:numLecteur ";
    requete += "AND lecteur.num_lieu = lieu.num_lieu LIMIT 1";
    query->prepare(requete);
    query->bindValue(":numLecteur", numLecteur);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL lieu du lecteur" << endl;
         return -1;
    }

    //récuperation de la valeur
    query->next();
    int numLieu = query->value(0).toInt();
    legende = query->value(1).toString();

    return numLieu;
}

///////////////////////
// voir état lecteur (co ou déco)
bool Bdd::getEtatLect(int numLecteur){

    //avec le numéro de lecteur obtenir l'état du lecteur (connecté ou non)
    requete = "SELECT estConnecte ";
    requete += "FROM lecteur ";
    requete += "WHERE num_lecteur=:numLecteur";
    query->prepare(requete);
    query->bindValue(":numLecteur", numLecteur);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL etat lecteur" << endl;
         return -1;
    }

    query->next();
    int etat = query->value(0).toInt();
    if (etat == 0)
        return false;

return true;
}



/*-----------------------------------*
 * Méthode pour obtenir l'identité   *
 * de la personne possédant le badge *
 *-----------------------------------*/
int Bdd::badgeIdentite(int num_badge_i, QList<T_Personne> *listePersonne,QList<T_Badge> *listeBadge){

    int num_pers;
    //avec le numéro de badge obtenir l'identité de la personne
    requete = "SELECT A1.num_pers, A1.id_badge, A2.nom, A2.prenom, A2.societe, A2.email, A2.portable ";
    requete += "FROM badge A1, personne A2 ";
    requete += "WHERE A1.num_pers=A2.num_pers AND A1.num_badge=:num_badge_i";
    query->prepare(requete);
    query->bindValue(":num_badge_i", num_badge_i);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL identite badge" << endl;
         return -1;
    }

    //allocation pointeur
    this->pPersonne = new T_Personne;
    this->pBadge = new T_Badge;

    if (query->size() > 0){
        query->next();
        num_pers = query->value(0).toInt();

        //ajout liste
        this->pBadge->id_badge = query->value(1).toString();
        this->pPersonne->nom = query->value(2).toString();
        this->pPersonne->prenom = query->value(3).toString();
        this->pPersonne->societe = query->value(4).toString();
        this->pPersonne->email = query->value(5).toString();
        this->pPersonne->portable = query->value(6).toString();
        listePersonne->append(*pPersonne);
        listeBadge->append(*pBadge);

        delete this->pPersonne;
        delete this->pBadge;

        return num_pers;
    }else{
        return -1;
    }
}

/*--------------------------------------*
 * Méthode pour obtenir les coordonnées *
 * d'un lieu en fonction d'un lecteur   *
 * et d'une vue                         *
 *--------------------------------------*/
/*bool Bdd::getPositionLieu(int num_vue, int num_lecteur, QList<T_TuplePositionLieu> *positionLieu){

    //avec le numéro de lecteur et la vue obtenir la position lieu
    requete = "SELECT DISTINCT A1.xA, A1.yA, A1.xB, A1.yB ";
    requete += "FROM representationLieuSurVue A1 ";
    requete += "WHERE A1.num_vue=:num_vue AND A1.num_lieu=:num_lecteur";
    query->prepare(requete);
    query->bindValue(":num_vue", num_vue);
    query->bindValue(":num_lecteur", num_lecteur);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL position lieu" << endl;
         return false;
    }

    //allocation pointeur
    this->pTuplePositionLieu = new T_TuplePositionLieu();

    //réponse requête
    while(query->next()){
        int xA = query->value(0).toInt();
        int yA = query->value(1).toInt();
        int xB = query->value(2).toInt();
        int yB = query->value(3).toInt();

        //ajout sur liste
        this->pTuplePositionLieu->xA = xA;
        this->pTuplePositionLieu->yA = yA;
        this->pTuplePositionLieu->xB = xB;
        this->pTuplePositionLieu->yB = yB;

        positionLieu->append(*pTuplePositionLieu);
    }

    delete this->pTuplePositionLieu;

    return true;
}
*/
//////////////////////////////////////
/*** METHODE pour OBTENIR VUE MAX ***/
int Bdd::getVueMax(){

    //requête
    if(!query->exec("SELECT COUNT(*) FROM vue")){
        qDebug() << "Erreur requete SQL" << endl; //<< database->lastError() << endl;
        //test si problème lors de l'envoi de la requete
    }

    int vueMax=0;
    //réponse requête
    while(query->next()){
        vueMax = query->value(0).toInt();
    }
    return vueMax;
}
////////////////////////////////////////
/*** obtenir sens de passage (type de zone)
 * montée, descentee, contigu ****/
int Bdd::getSensMonter(int numZone){

    //avec le numéro de lecteur, obtenir le type de zone
    requete = "SELECT DISTINCT A1.sensMonter ";
    requete += "FROM zone A1 ";
    requete += "WHERE A1.num_zone=:numZone";
    query->prepare(requete);
    query->bindValue(":numZone", numZone);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL sens de passage (zone)" << endl;
         return -1;
    }
    //réponse requête
    query->next();
    int sensMonter = query->value(0).toInt();
    return sensMonter;

}


/////////////////////////////////////////////////////////
/*** METHODE pour OBTENIR VUE en FONCTION du n° LECTEUR ***/
// repris le 13/11/2017
bool Bdd::getVueFctLect(int numZone, QList<T_TupleLecteurS> *listeLecteur){

    //avec le numéro obtenu, obtenir la vue
    requete = "SELECT DISTINCT A2.num_vue ";
    requete += "FROM representationLieuSurVue A2 ";
    requete += "WHERE num_zone=:numZone";
    query->prepare(requete);
    query->bindValue(":numLecteur", numZone);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL vue lecteur" << endl;
    }

    //allocation pointeur
    this->pTupleLecteurS = new T_TupleLecteurS();
    //réponse requête
    while(query->next()){
         int num_vue = query->value(0).toInt();
        //ajout sur liste
         this->pTupleLecteurS->num_vue = num_vue;
         listeLecteur->append(*pTupleLecteurS);
    } // while
    delete this->pTupleLecteurS;
    return true;
}
///////////////////////////////////////////////////////////////////
/*** METHODE OBTENIR VUE et POSITION en FONCTION du n° LECTEUR ***/
bool Bdd::getVuePosFctLect(int numLecteur, QList<T_TupleLecteurE> *listeLecteur){

    //avec le numéro obtenu, obtenir la vue et la position (x,y)
    requete = "SELECT DISTINCT A2.num_vue, A2.x, A2.y ";
    requete += "FROM lecteur A1, representationLieuSurVue A2 ";
    requete += "WHERE A1.num_lieu = A2.num_lieu AND A1.num_lecteur=:numLecteur AND A2.num_zone<10";
    query->prepare(requete);
    query->bindValue(":numLecteur", numLecteur);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL vue/position lecteur" << endl;
    }

    //allocation pointeur
    this->pTupleLecteurE = new T_TupleLecteurE();

    //réponse requête
    while(query->next()){
         int num_vue = query->value(0).toInt();
         int x = query->value(1).toInt();
         int y = query->value(2).toInt();

        //ajout sur liste
         this->pTupleLecteurE->num_vue = num_vue;
         this->pTupleLecteurE->x = x;
         this->pTupleLecteurE->y = y;
         listeLecteur->append(*pTupleLecteurE);
    }

    delete this->pTupleLecteurE;

    return true;
}
/////////////////////////////////////////
/*** METHODE OBTENIR TOUTES LES VUES ***/
bool Bdd::getVue(QList<T_TupleOnglet> *listeOnglet){

    // requête
    if(!query->exec("SELECT * FROM vue")){
        qDebug() << "Erreur requete SQL" << endl << database.lastError() << endl;
        //test si problème lors de l'envoi de la requete
    }

    //allocation pointeur
    this->pTupleOnglet = new T_TupleOnglet;

    //réponse requete
    while(query->next()){
        int num_vue = query->value(0).toInt();
        QString legende = query->value(1).toString();
        QString image = query->value(2).toString();

        //ajout liste
        this->pTupleOnglet->num_vue = num_vue;
        this->pTupleOnglet->legende = legende;
        this->pTupleOnglet->image = image;
        listeOnglet->append(*pTupleOnglet);

    }

    delete this->pTupleOnglet;

    return true;
}
////////////////////////////////////////////
/*** METHODE est-ce qu'un badge existe ?***/
bool Bdd::badgeExiste(int num_badge){

    //requête
    requete = "SELECT * FROM badge WHERE num_badge=:numBadge ";
    query->prepare(requete);
    query->bindValue(":numBadge", num_badge);

    if (!query->exec()){
        qDebug("Erreur MySQL badgeExiste");
        return false;
    }

    //réponse requête
    //astuce : connaitre son nombre de ligne renvoyé permet de savoir si plusieurs badges correspondent à la requete
    int nbLigne = query->size();

    //le badge existe
    if(nbLigne == 1){
        query->next();
        return true;
    }else{

        //le badge n'existe pas
        return false;
    }


}


///             CAMERA          ///
/*** OBTENIR CAMERAS EXISTANTES ***/
bool Bdd::getCameraExistante(QList<T_Camera> * listeCamera)
{

    //requete
    requete = "SELECT Adresse_IP, URL, Port_TCP, Type_Camera, Num_Camera, Num_Lieu, Num_Vue FROM cameras";
    query->prepare(requete);
    if(!query->exec()) {
        qDebug() << "Erreur requete SQL liste cameras" << database.lastError() << endl;
        return false;
    }

    //allocation requete
    this->pCamera = new T_Camera;

    //reponse requete
    while(query->next()) {
        QString Adresse_IP = query->value(0).toString();
        QString URL = query->value(1).toString();
        QString Port_TCP = query->value(2).toString();
        QString Type_Camera = query->value(3).toString();
        QString Num_Camera = query->value(4).toString();
        QString Num_Lieu = query->value(5).toString();
        QString Num_Vue = query->value(6).toString();

        //ajout liste
        this->pCamera->Adresse_IP = Adresse_IP;
        this->pCamera->URL = URL;
        this->pCamera->Port_TCP = Port_TCP;
        this->pCamera->Type_Camera = Type_Camera;
        this->pCamera->Num_Camera = Num_Camera;
        this->pCamera->Num_Lieu = Num_Lieu;
        this->pCamera->Num_Vue = Num_Vue;

        listeCamera->append(*pCamera);
    }

    delete this->pCamera;
    return true;
}

/*** SELECTION CAMERA | estActif = 1 ***/
bool Bdd::setChoixCamera(QString selectedCam)
{

    //requete
    requete = "UPDATE cameras ";
    requete += "SET estActif = 1 ";
    requete += "WHERE Num_Camera=:selectedCam";

    query->prepare(requete);
    query->bindValue(":selectedCam", selectedCam);


    if(!query->exec()) {
        qDebug() << "Erreur requete SQL choix camera" << database.lastError() << endl;
        return false;
    }
    return true;
}

/*** estActif = 0 ***/
bool Bdd::setInactif(QString inactif)
{

    //requete
    requete = "UPDATE cameras ";
    requete += "SET estActif = 0 ";

    query->prepare(requete);
    query->bindValue(":selectedCam", inactif);


    if(!query->exec()) {
        qDebug() << "Erreur requete SQL choix camera" << database.lastError() << endl;
        return false;
    }
    return true;
}

/*** OBTENIR NUM CAMERA ***/
int Bdd::getNumCamera(QString camera)
{

    // Requete
    requete = "SELECT Num_Camera ";
    requete += "FROM cameras ";
    requete += "WHERE Type_Camera=:cameras";

    query->prepare(requete);
    query->bindValue(":camera", camera);
    if(!query->exec()) {
        qDebug() << "Erreur requete SQL obtenir num camera" << endl << database.lastError() << endl;
        return -1;
    }

    // Reponse requete
    query->next();
    int numCam = query->value(0).toInt();
    return numCam;
}
