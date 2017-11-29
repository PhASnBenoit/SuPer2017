#include "bdd.h"
#include <QString>
#include <QSettings>

//////////////////////
/*** CONSTRUCTEUR ***/
Bdd::Bdd(QObject *parent) :
    QObject(parent)
{

    //creer objet avec paramètre chemin d'access et format de lecture
 /*   QSettings settings("SuPer.ini", QSettings::IniFormat);
    //prend valeur 1, si non trouver prend valeur 2
    QString ip = settings.value("Adresse/adresse", "192.168.2.70").toString();
    QString ndc = settings.value("Identifiants/NomDeCompte", "super2015").toString();
    QString mdp = settings.value("Identifiants/MotDePasse", "super2015").toString();
    QString NomBDD = settings.value("Identifiants/NomDeBase", "bdd_super").toString();*/
    //accès BDD
    database = QSqlDatabase::addDatabase("QMYSQL");
   /* database.setHostName(ip);
    database.setDatabaseName(NomBDD);
    database.setUserName(ndc);//QBytesArray contenant valeur décrypté
    database.setPassword(mdp);//QBytesArray contenant valeur décrypté*/
    database.setHostName("192.168.70.70");
    database.setDatabaseName("bdd_super2015");
    database.setUserName("super2015");//QBytesArray contenant valeur décrypté
    database.setPassword("super2015");//QBytesArray contenant valeur décrypté
    bool ok = database.open();
    if (!ok){
        this->accesBdd = false;
        qDebug() << database.lastError();
    }
    else{
        this->accesBdd = true;
    }
    query = new QSqlQuery;

}
/////////////////////
/*** DESTRUCTEUR ***/
Bdd::~Bdd(){
    delete query;
    database.close();
}

////////////////////AUTHENTIFICATION    ---- CHAMART////////////////////////////
////////
//obtenir la liste de login d'authentification
QList<QString> Bdd::getLogin()
{
    //requete
    requete = "SELECT login ";
    requete += "FROM super";

    //formater la requete
    query->prepare(requete);

    if(!query->exec())
    {
        qDebug() << "Erreur requete SQL obtenir login" << endl << database.lastError() << endl;
    }

    QList<QString> listeLogin;
    //réponse requete
    while(query->next())
    {
        //stocker les mdp dans une liste
        listeLogin.append(query->value(0).toString());
    }

    return listeLogin;
}

////
//obtenir liste de mot de passe
QList<QString> Bdd::getPassword()
{
    //requete
    requete = "SELECT password ";
    requete += "FROM super";

    //formater la requete
    query->prepare(requete);

    if(!query->exec())
    {
        //afficher erreur debegeur
        qDebug() << "Erreur requete SQL obtenir password" << endl << database.lastError() << endl;
    }

    QList<QString> listePassword;
    //réponse requete
    while(query->next())
    {
        //stocker les mdp dans une liste
        listePassword.append(query->value(0).toString());
    }

    return listePassword;
}

///
//affecter le password
bool Bdd::setPassword(QString password,QString selectedLogin)
{
    //requete
    requete = "UPDATE super ";
    requete += "SET password =:password ";
    requete += "WHERE login =:selectedLogin ";

    //preparer la requete SQL
    query->prepare(requete);

    //remplacer la valeur du password
    query->bindValue(":password", password);
    query->bindValue(":selectedLogin", selectedLogin);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif password " << database.lastError() << endl;
        return false;
    }
    return true;
}

///
//affecter le login
bool Bdd::setLogin(QString login,QString selectedLogin)
{
    requete = "UPDATE super ";
    requete += "SET login =:login ";
    requete += "WHERE login =:selectedLogin ";
    query->prepare(requete);
    query->bindValue(":login", login);
    query->bindValue(":selectedLogin", selectedLogin);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif login " << database.lastError() << endl;
        return false;
    }
    return true;
}



///////////////////////BADGES////////////////////////////////////////
//obtenir uniquement les numéros de personne qui n'apparraissent pas dans badge
bool Bdd::getPersonneALier(QList<T_Personne> *listePersonne){

    // requête
    requete = "SELECT nom, prenom, societe, dateDebut, dateFin FROM personne WHERE num_pers NOT IN(SELECT num_pers FROM badge) AND num_pers !=1";

    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL personneLier" << endl << database.lastError() << endl;
        return false;
    }

    if(query->size() == 0)
        return false;

    //allocation pointeur
    this->pPersonne = new T_Personne;

    //réponse requete
    while(query->next()){
        QString nom = query->value(0).toString();
        QString prenom = query->value(1).toString();
        QString societe = query->value(2).toString();
        QString dateDebut = query->value(3).toString();
        QString dateFin = query->value(4).toString();

        //ajout liste
        this->pPersonne->nom = nom;
        this->pPersonne->prenom = prenom;
        this->pPersonne->societe = societe;
        this->pPersonne->dateDebut = dateDebut;
        this->pPersonne->dateFin = dateFin;
        listePersonne->append(*pPersonne);

    }

    delete this->pPersonne;

    return true;
}
////////
//obtenir le numéro de la personne
int Bdd::getNumPersonne(QString personne){
    //requete
    requete = "SELECT num_pers ";
    requete += "FROM personne ";
    requete += "WHERE nom=:personne";
    query->prepare(requete);
    query->bindValue(":personne", personne);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL obtenir num personne" << endl << database.lastError() << endl;
        return -1;
    }
    //réponse requete
    query->next();
    int numPersonne = query->value(0).toInt();
    return numPersonne;
}
////////
//mettre en oeuvre l'affectation
bool Bdd::setLier(int num_badge, int num_pers){
    //requete
    requete = "UPDATE badge ";
    requete += "SET num_pers=:num_pers, estActif=0 ";
    requete += "WHERE num_badge=:num_badge";

    query->prepare(requete);
    query->bindValue(":num_badge", num_badge);
    query->bindValue(":num_pers", num_pers);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL affectation" << endl << database.lastError() << endl;
        return false;
    }
    return true;

}
bool Bdd::AddBadge(int numBadge, QString id_badge, QString dateService, QString datePile){

    requete = "INSERT INTO badge (num_badge, num_pers, id_badge,  dateMiseEnService, dateChangePile, estActif) ";
    requete += "VALUES (:numBadge, 1, :id_badge, :dateService, :datePile, 0)";

    query->prepare(requete);
    query->bindValue(":numBadge", numBadge);
    query->bindValue(":id_badge", id_badge);
    query->bindValue(":dateService", dateService);
    query->bindValue(":datePile", datePile);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL création" << endl << database.lastError() << endl;
        return false;
    }
    return true;
}

bool Bdd::setModBadge(int num_badge, QString id_badge, QString dateMiseEnService, QString dateChangePile)// Modifier personne

    {
    //requete
    requete = "UPDATE badge ";
    requete += "SET id_badge=:id_badge, dateMiseEnService=:dateMiseEnService, dateChangePile=:dateChangePile ";
    requete += "WHERE num_badge=:num_badge";

    query->prepare(requete);
    query->bindValue(":num_badge", num_badge);
    query->bindValue(":id_badge", id_badge);
    query->bindValue(":dateMiseEnService", dateMiseEnService);
    query->bindValue(":dateChangePile", dateChangePile);
        if(!query->exec())
        {
            qDebug() << "Erreur requete SQL modifier badge " << database.lastError() << endl;
            return false;
        }
    return true;

}

////////
//désaffecter badge à une personne
bool Bdd::setDelier(int num_badge){
    //requete
    requete = "UPDATE badge ";
    requete += "SET num_pers = 1 ";
    requete += "WHERE num_badge = :num_badge";
    query->prepare(requete);
    query->bindValue(":num_badge", num_badge);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL desaffectation " << database.lastError() << endl;
        return false;
    }
    return true;

}

//suppression d'un badge a test 02-02-15
bool Bdd::setSuppbadge(int num_badge){
    requete = "DELETE FROM badge ";
    requete += "WHERE num_badge=:num_badge";
    query->prepare(requete);
    query->bindValue(":num_badge", num_badge);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL suppression " << database.lastError() << endl;
        return false;
    }
    return true;

}


//obtenir numéro de badge
bool Bdd::getNumBadge(QString *badgeExistant){

    badgeExistant->fromUtf8("");

    //requête
    requete = "SELECT num_badge ";
    requete += "FROM badge";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL badge existant" << endl << database.lastError() << endl;
        return false;
    }

    //bdd vide
    if(query->size() == 0)
        return false;

    //réponse requete
    while(query->next()){
        badgeExistant->append(query->value(0).toString());
        badgeExistant->append("; ");
    }
    return true;
}



////////
int Bdd::getNumPers(int num_badge){

    requete = "SELECT num_pers FROM badge ";
    requete += "WHERE num_badge=:num_badge";
    query->prepare(requete);
    query->bindValue("num_badge", num_badge);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL obtenir num personne" << endl << database.lastError() << endl;
        return -1;
    }
    //réponse requete
    query->next();
    int numPersonne = query->value(0).toInt();
    return numPersonne;
}
//obtenir badges lié
bool Bdd::getBadgeLies(QList<T_Badge> *listeBadgeLier){

    requete = "SELECT num_badge, id_badge, dateMiseEnService, dateChangePile ";
    requete +="FROM badge ";
    requete +="WHERE num_pers != 1 AND num_badge !=0";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL badge à délier " << endl << database.lastError() << endl;
        return false;
    }
    //allocation pointeur
    this->pBadgeLier = new T_Badge;

    //réponse requête
    while(query->next()){
        QString num_badge = query->value(0).toString();
        QString id_badge = query->value(1).toString();
        QString dateMiseEnService = query->value(2).toString();
        QString dateChangePile = query->value(3).toString();
        //ajout liste
        this->pBadgeLier->numBadge = num_badge;
        this->pBadgeLier->id_badge = id_badge;
        this->pBadgeLier->dateMiseEnService = dateMiseEnService;
        this->pBadgeLier->dateChangePile = dateChangePile;
        listeBadgeLier->append(*pBadgeLier);
    }

    delete this->pBadgeLier;
    return true;
}
//obtenir badges non lié
bool Bdd::getBadgeNonActif(QList<T_Badge> *listeBadge){

   //test si il y a des badge avec num_pers = 1
    requete = "SELECT COUNT(num_badge) FROM badge WHERE num_pers=1";
    query->prepare(requete);
    query->exec();
    query->next();
    int CompteurBadge = query->value(0).toInt();

    if(CompteurBadge !=0){

        // requête
        requete = "SELECT num_badge, id_badge, dateMiseEnService, dateChangePile FROM badge WHERE num_pers = 1 AND num_badge !=0";
        query->prepare(requete);
        if(!query->exec())
        {
            qDebug() << "Erreur requete SQL badgenonactif" << endl << database.lastError() << endl;
            return false;
        }//if

        //allocation pointeur
        this->pBadge = new T_Badge;

            //réponse requête
            while(query->next()){
                QString num_badge = query->value(0).toString();
                QString id_badge = query->value(1).toString();
                QString dateMiseEnService = query->value(2).toString();
                QString dateChangePile = query->value(3).toString();

                //ajout liste
                this->pBadge->numBadge = num_badge;
                this->pBadge->id_badge = id_badge;
                this->pBadge->dateMiseEnService = dateMiseEnService;
                this->pBadge->dateChangePile = dateChangePile;
                listeBadge->append(*pBadge);
            }//while

        delete this->pBadge;


    }//if

    return true;
}
/////////////////////////////////END BADGES/////////////////////////////////////////
//obtenir la liste de l'historique des événements
bool Bdd::getLog(QList<T_Log> * listeLog){
    // requête
    //obtenir date, legende, nom, prénom, société (pour historique)
    requete = "SELECT A1.dateLog, A1.legende, A2.nom, A2.prenom, A2.societe, A1.login, A1.num_badge ";
    requete += "FROM log A1, personne A2, badge A3 ";
    requete += "WHERE A1.num_badge=A3.num_badge AND A3.num_pers=A2.num_pers";
    query->prepare(requete);
    if (!query->exec()){
        qDebug() << "Erreur requete SQL historique evenement" << endl << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pLog = new T_Log;

    //bdd vide
    if(query->size() == 0)
        return false;

    //réponse requete
    while(query->next()){
        QString date_log = query->value(0).toString();
        QString legende = query->value(1).toString();
        QString nom = query->value(2).toString();
        QString prenom = query->value(3).toString();
        QString societe = query->value(4).toString();
        QString login = query->value(5).toString();
        QString num_badge = query->value(6).toString();


        //ajout liste
        this->pLog->date = date_log;
        this->pLog->legende = legende;
        this->pLog->nom = nom;
        this->pLog->prenom = prenom;
        this->pLog->societe = societe;
        this->pLog->login = login;
        this->pLog->numBadge = num_badge;
        listeLog->append(*pLog);
    }

    delete this->pLog;

    return true;
}
/////////
//sauver les log de la connexion
bool Bdd::setLogConnection(QString login)
{
    QString dateLog = QDateTime::currentDateTime().toString();

    QString legende = "Nouvelle connexion dans CONFIGURATION";

    requete= "INSERT INTO log (login, dateLog, legende) ";
    requete += "VALUES (:login, :dateLog, :legende)";

    query->prepare(requete);
    query->bindValue(":login",login);
    query->bindValue(":dateLog",dateLog);
    query->bindValue(":legende",legende);

    if(!query->exec())
    {
        qDebug() << "Erreur requete SQL ajout LogConnection" << endl;
        return false;
    }

    return true;
}

/////////
//supprimer l'historique des événements
bool Bdd::removeLog(){
    //requête
    //supprimer tout les tuples de la table log
    requete = "DELETE FROM log";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL suppression historique" << endl << database.lastError() << endl;
        return false;
    }
    return true;

}

/////////

////////
//ajouter une vue
bool Bdd::setVue(QString numVue, QString legende, QString image){
    //requete
    requete = "INSERT INTO vue (num_vue, legende, image) ";
    requete += "VALUES (:numVue, :legende, :image)";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    query->bindValue(":legende", legende);
    query->bindValue(":image", image);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter vue " << database.lastError() << endl;
        return false;
    }
    return true;

}
/////
//obtenir vue existante (QList)
bool Bdd::getVueExistant(QList<T_Vue> *listeVue){
    //requête
    requete = "SELECT num_vue, legende, image ";
    requete += "FROM vue";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste vue " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pVue = new T_Vue;

    //réponse requête
    while(query->next()){
        QString num_vue = query->value(0).toString();
        QString legende = query->value(1).toString();
        QString image = query->value(2).toString();

        //ajout liste
        this->pVue->numVue = num_vue;
        this->pVue->legende = legende;
        this->pVue->image = image;

        listeVue->append(*pVue);
    }

    delete this->pVue;
    return true;
}
//////
//modifier vue
bool Bdd::addModVue(QString legendeActuelle, QString numVue, QString legende, QString image){
    //requete
    requete = "UPDATE vue ";
    requete += "SET num_vue = :numVue, legende = :legende, image = :image ";
    requete += "WHERE legende = :legendeActuelle";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    query->bindValue(":legende", legende);
    query->bindValue(":image", image);
    query->bindValue(":legendeActuelle", legendeActuelle);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif vue " << database.lastError() << endl;
        return false;
    }
    return true;
}
//////
//supprimer vue
bool Bdd::setSuppVue(QString numVue){
    //requete
    requete = "DELETE FROM vue ";
    requete += "WHERE num_vue = :numVue";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp vue " << database.lastError() << endl;
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//obtenir liste des lieux existants
bool Bdd::getLieuExistant(QList<T_Lieu> *listeLieu){
    //requête
    requete = "SELECT num_lieu, legende ";
    requete += "FROM lieu";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste lieu " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pLieu = new T_Lieu;

    //réponse requête
    while(query->next()){
        QString num_lieu = query->value(0).toString();
        QString legende = query->value(1).toString();

        //ajout liste
        this->pLieu->numLieu = num_lieu;
        this->pLieu->legende = legende;

        listeLieu->append(*pLieu);
    }

    delete this->pLieu;
    return true;
}
//////////
//ajouter lieu
bool Bdd::setLieu(QString numLieu, QString legende){
    //requete
    requete = "INSERT INTO lieu (num_lieu, legende) ";
    requete += "VALUES (:numLieu, :legende)";
    query->prepare(requete);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":legende", legende);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter lieu " << database.lastError() << endl;
        return false;
    }
    return true;
}
//////////
//modifier lieu
bool Bdd::addModLieu(QString legendeActuelle, QString numLieu, QString legende){
    //requete
    requete = "UPDATE lieu ";
    requete += "SET num_lieu = :numLieu, legende = :legende ";
    requete += "WHERE legende = :legendeActuelle";
    query->prepare(requete);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":legende", legende);
    query->bindValue(":legendeActuelle", legendeActuelle);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif lieu " << database.lastError() << endl;
        return false;
    }
    return true;
}
////////
//supprimer lieu
bool Bdd::setSuppLieu(QString numLieu){
    //requete
    requete = "DELETE FROM lieu ";
    requete += "WHERE num_lieu = :numLieu";
    query->prepare(requete);
    query->bindValue(":numLieu", numLieu);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp lieu " << database.lastError() << endl;
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//obtenir zone existante
bool Bdd::getZoneExistant(QList<T_Zone> * listeZone){
    //requête
    requete = "SELECT num_zone, num_lieu, sensMonter, legende ";
    requete += "FROM zone";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste zone " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pZone = new T_Zone;

    //réponse requête
    while(query->next()){
        QString num_zone = query->value(0).toString();
        QString num_lieu = query->value(1).toString();
        QString sensMonter = query->value(2).toString();
        QString legende = query->value(3).toString();

        //ajout liste
        this->pZone->numZone = num_zone;
        this->pZone->numLieu = num_lieu;
        this->pZone->sensMonter = sensMonter;
        this->pZone->legende = legende;

        listeZone->append(*pZone);
    }

    delete this->pZone;
    return true;
}
////////
//ajouter zone
bool Bdd::setZone(QString numZone, int numLieu, QString sensMonter, QString legende){
    //requete
    requete = "INSERT INTO zone (num_zone, num_lieu, sensMonter, legende) ";
    requete += "VALUES (:numZone, :numLieu, :sensMonter, :legende)";
    query->prepare(requete);
    query->bindValue(":numZone", numZone);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":sensMonter", sensMonter);
    query->bindValue(":legende", legende);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter zone " << database.lastError() << endl;
        return false;
    }
    return true;
}
///////
//obtenir numéro du lieu
int Bdd::getNumLieu(QString legende){
    //requete
    requete = "SELECT num_lieu ";
    requete += "FROM lieu ";
    requete += "WHERE legende=:legende";
    query->prepare(requete);
    query->bindValue(":legende", legende);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL obtenir num lieu " << database.lastError() << endl;
        return -1;
    }
    //réponse requete
    query->next();
    int numLieu = query->value(0).toInt();
    return numLieu;
}
////////
//modifier zone
bool Bdd::addModZone(QString numZone, QString numLieuActuel, QString numLieu, QString sensMonter, QString legende){
    //requete
    requete = "UPDATE zone ";
    requete += "SET num_lieu = :numLieu, sensMonter = :sensMonter, legende = :legende ";
    requete += "WHERE num_lieu = :numLieuActuel AND num_zone = :numZone";
    query->prepare(requete);
    query->bindValue(":numZone", numZone);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":sensMonter", sensMonter);
    query->bindValue(":legende", legende);
    query->bindValue(":numLieuActuel", numLieuActuel);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif zone " << database.lastError() << endl;
        return false;
    }
    return true;
}
///////
//supprimer zone
bool Bdd::setSuppZone(QString numZone){
    //requete
    requete = "DELETE FROM zone ";
    requete += "WHERE num_zone = :numZone";
    query->prepare(requete);
    query->bindValue(":numZone", numZone);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp zone " << database.lastError() << endl;
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////
//obtenir legende lieu
QString Bdd::getLegendeLieu(QString numLieu){
    //requête
    requete = "SELECT legende ";
    requete += "FROM lieu ";
    requete += "WHERE num_lieu = :numLieu";
    query->prepare(requete);
    query->bindValue(":numLieu", numLieu);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL get legende lieu " << database.lastError() << endl;
        return "erreur";
    }
    query->next();
    QString legende = query->value(0).toString();
    return legende;
}
//obtenir représentation
bool Bdd::getRepresentation(QList<T_Representation> *listeRepresentation, QString numVue, QString numZone){
    //requête
    requete = "SELECT A1.x, A1.y, A1.xA, A1.yA, A1.xB, A1.yB ";
    requete += "FROM representationLieuSurVue A1 ";
    requete += "WHERE num_vue=:numVue AND num_zone=:numZone";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    query->bindValue(":numZone", numZone);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste representation " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pRepresentation = new T_Representation;

    //bdd vide
    if(query->size() == 0)
        return false;

    //réponse requête
    while(query->next()){
        QString x = query->value(0).toString();
        QString y = query->value(1).toString();
        QString xA = query->value(2).toString();
        QString yA = query->value(3).toString();
        QString xB = query->value(4).toString();
        QString yB = query->value(5).toString();

        //ajout liste
        this->pRepresentation->x = x;
        this->pRepresentation->y = y;
        this->pRepresentation->xA = xA;
        this->pRepresentation->yA = yA;
        this->pRepresentation->xB = xB;
        this->pRepresentation->yB = yB;

        listeRepresentation->append(*pRepresentation);
    }

    delete this->pRepresentation;
    return true;
}
//ajouter positions
bool Bdd::setPosition(QString numVue, QString numZone, QString numLieu, QString xA, QString yA, QString xB, QString yB, QString x, QString y){
    //requête
    requete = "INSERT INTO representationLieuSurVue (num_vue, num_lieu, num_zone, x, y, xA, yA, xB, yB) ";
    requete += "VALUES (:numVue, :numLieu, :numZone, :x, :y, :xA, :yA, :xB, :yB)";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":numZone", numZone);
    query->bindValue(":x", x);
    query->bindValue(":y", y);
    query->bindValue(":xA", xA);
    query->bindValue(":yA", yA);
    query->bindValue(":xB", xB);
    query->bindValue(":yB", yB);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter positions " << database.lastError() << endl;
        return false;
    }
    return true;
}
//modifier positions
bool Bdd::addModPosition(QString numVue, QString numZone, QString numLieu, QString xA, QString yA, QString xB, QString yB, QString x, QString y){
    //requete
    requete = "UPDATE representationLieuSurVue ";
    requete += "SET x=:x, y=:y, xA=:xA, yA=:yA, xB=:xB, yB=:yB ";
    requete += "WHERE num_vue=:numVue AND num_lieu=:numLieu AND num_zone=:numZone";
    query->prepare(requete);
    query->bindValue(":x", x);
    query->bindValue(":y", y);
    query->bindValue(":xA", xA);
    query->bindValue(":yA", yA);
    query->bindValue(":xB", xB);
    query->bindValue(":yB", yB);
    query->bindValue(":numVue", numVue);
    query->bindValue(":numLieu", numLieu);
    query->bindValue(":numZone", numZone);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif positions " << database.lastError() << endl;
        return false;
    }
    return true;
}
//recherche zones liées à une vue
bool Bdd::getZoneLierVue(QList<T_Representation> *listeRepresentation, QString numVue){
    //requête
    requete = "SELECT num_zone ";
    requete += "FROM representationLieuSurVue ";
    requete += "WHERE num_vue=:numVue ";
    requete += "ORDER BY num_zone ASC";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste zone lier vue " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pRepresentation = new T_Representation;

    //réponse requête
    while(query->next()){
        QString num_zone = query->value(0).toString();

        //ajout liste
        this->pRepresentation->numZone = num_zone;
        listeRepresentation->append(*pRepresentation);
    }

    delete this->pRepresentation;
    return true;
}
//supprimer position
bool Bdd::setSuppPosition(QString numVue, QString numZone){
    //requete
    requete = "DELETE FROM representationLieuSurVue ";
    requete += "WHERE num_vue = :numVue AND num_zone=:numZone";
    query->prepare(requete);
    query->bindValue(":numVue", numVue);
    query->bindValue(":numZone", numZone);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp position " << database.lastError() << endl;
        return false;
    }
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//obtenir tempo
void Bdd::getTempo(int *tempoM, int *tempoR){
    //requête
    requete = "SELECT tempoM, tempoR ";
    requete += "FROM super ";
    requete += "LIMIT 1";
    query->prepare(requete);
    if(!query->exec()){
         qDebug() << "Erreur requete SQL obtenir tempo " << database.lastError() << endl;
    }

    //réponse requête
    query->next();
    *tempoM = query->value(0).toInt();
    *tempoR = query->value(1).toInt();

}
//modifier tempo
bool Bdd::setTempo(QString tempoMouv, QString tempoRecep){
    //requête
    requete = "UPDATE super ";
    requete += "SET tempoM=:tempoMouv, tempoR=:tempoRecep ";
    requete += "WHERE config=1";
    query->prepare(requete);
    query->bindValue("tempoMouv", tempoMouv);
    query->bindValue("tempoRecep", tempoRecep);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif tempo " << database.lastError() << endl;
        return false;
    }
    return true;
}













/////////////////////////////////////FONCTIONS POUR LES LECTEURS//////////////////////
///
////////OBTENIR LES LECTEURS EXISTANT
bool Bdd::getLecteurExistant(QList<T_Lecteur> * listeLecteurExistants){

    //requÃªte
    requete = "SELECT num_lecteur, num_lieu, ip ";
    requete += "FROM lecteur";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste lecteurs " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pLecteur = new T_Lecteur;

    //réponse requÃªte
    while(query->next()){
        QString num_lecteur = query->value(0).toString();
        QString num_lieu = query->value(1).toString();
        QString ip = query->value(2).toString();
        //QString estConnecte = query->value(3).toString();

        //ajout liste
        this->pLecteur->numLecteur = num_lecteur;
        this->pLecteur->numLieuLecteur = num_lieu;
        this->pLecteur->ipLecteur = ip;

        listeLecteurExistants->append(*pLecteur);
    }

    delete this->pLecteur;
    return true;


}

/////AJOUTER UN LECTEUR
bool Bdd::addLecteur(QString numLecteur, QString numLieuLecteur, QString ipLecteur){

    //requete
    requete = "INSERT INTO lecteur (num_lecteur, num_lieu, ip, estConnecte) ";
    requete += "VALUES (:numLecteur, :numLieuLecteur, :ipLecteur, 0)";
    query->prepare(requete);
    query->bindValue(":numLecteur", numLecteur);
    query->bindValue(":numLieuLecteur", numLieuLecteur);
    query->bindValue(":ipLecteur", ipLecteur);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter lecteur " << database.lastError() << endl;
        return false;
    }
    return true;

}

//////MODIFIER UN LECTEUR
bool Bdd::setLecteur(QString numLecteur, QString numLieuLecteur, QString ipLecteur){

    //requete
    requete = "UPDATE lecteur ";
    requete += "SET num_lieu=:numLieuLecteur, ip=:ipLecteur ";
    requete += "WHERE num_lecteur=:numLecteur";

    query->prepare(requete);
    query->bindValue(":numLecteur", numLecteur);
    query->bindValue(":numLieuLecteur", numLieuLecteur);
    query->bindValue(":ipLecteur", ipLecteur);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL modif lecteur " << database.lastError() << endl;
        return false;
    }
    return true;
}


/////SUPPRIMER UN LECTEUR
bool Bdd::setSuppLecteur(QString numLecteur){
    //requete
    requete = "DELETE FROM lecteur ";
    requete += "WHERE num_lecteur=:numLecteur";
    query->prepare(requete);
    query->bindValue("numLecteur", numLecteur);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp lecteur " << database.lastError() << endl;
        return false;
    }
    return true;
}

/////////////////////////////////////FONCTIONS POUR LES PERSONNES/////////////////////////////////////
/////OBTENIR PERSONNE EXISTANTES
bool Bdd::getPersonneExistant(QList<T_Personne> * listePersonnesExistantes){

    //requÃªte
    requete = "SELECT num_pers, nom, prenom, societe, dateDebut, dateFin, email, portable ";
    requete += "FROM personne WHERE num_pers !=1";
    query->prepare(requete);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL liste personne " << database.lastError() << endl;
        return false;
    }

    //allocation pointeur
    this->pPersonne = new T_Personne;

    //réponse requÃªte
    while(query->next()){
        QString num_pers = query->value(0).toString();
        QString nom = query->value(1).toString();
        QString prenom = query->value(2).toString();
        QString societe = query->value(3).toString();
        QString dateDebut = query->value(4).toString();
        QString dateFin = query->value(5).toString();
        QString email = query->value(6).toString();
        QString portable = query->value(7).toString();

        //ajout liste
        this->pPersonne->num_pers = num_pers;
        this->pPersonne->nom = nom;
        this->pPersonne->prenom = prenom;
        this->pPersonne->societe = societe;
        this->pPersonne->dateDebut = dateDebut;
        this->pPersonne->dateFin = dateFin;
        this->pPersonne->portable = portable;
        this->pPersonne->email = email;

        listePersonnesExistantes->append(*pPersonne);
    }

    delete this->pPersonne;
    return true;

}
////////OBTENIR UN NOUVEAU NUMERO DE PERSONNE
int Bdd::getNouveauNumPersonne()
{
    requete = "SELECT MAX(num_pers)+1 FROM personne";
    query->prepare(requete);

    if(!query->exec())
    {
        qDebug() << "Erreur requete SQL getNouveauNumPersonne " << database.lastError() << endl;
    }

    query->next();

    int num_pers=query->value(0).toInt();

    return num_pers;
}
////////AJOUTER UNE NOUVELLE PERSONNE
bool Bdd::addPersonne(int num_pers, QString nom, QString prenom, QString email, QString portable, QString societe, QString dateDebut, QString dateFin)
{
    //requete
    requete = "INSERT INTO personne (num_pers, nom, prenom, societe, dateDebut, dateFin, email, portable) ";
    requete += "VALUES (:num_pers, :nom, :prenom, :societe, :dateDebut, :dateFin, :email, :portable)";

    query->prepare(requete);

    query->bindValue(":num_pers", num_pers);
    query->bindValue(":nom", nom);
    query->bindValue(":prenom", prenom);
    query->bindValue(":societe", societe);
    query->bindValue(":dateDebut", dateDebut);
    query->bindValue(":dateFin", dateFin);
    query->bindValue(":portable", portable);
    query->bindValue(":email", email);
    if(!query->exec()){
        qDebug() << "Erreur requete SQL ajouter personne " << database.lastError() << endl;
        return false;
    }
    return true;

}
////////MODIFIER UNE PERSONNE
bool Bdd::setModPersonne(QString selectedNom,QString nom, QString prenom, QString email, QString portable, QString societe, QString dateDebut, QString dateFin) // Modifier personne
{
    //obtention du num de la personne car inmodifiable
    int iNum = getNumPersonne(nom);

    //requete
    requete = "UPDATE personne ";
    requete += "SET num_pers=:num_pers, nom=:nom, prenom=:prenom, societe=:societe, dateDebut=:dateDebut, dateFin=:dateFin, email=:email, portable=:portable ";
    requete += "WHERE nom=:selectedNom";

    query->prepare(requete);

    query->bindValue(":selectedNom", selectedNom);
    query->bindValue(":num_pers",iNum);
    query->bindValue(":nom", nom);
    query->bindValue(":prenom", prenom);
    query->bindValue(":societe", societe);
    query->bindValue(":dateDebut", dateDebut);
    query->bindValue(":dateFin", dateFin);
    query->bindValue(":email", email);
    query->bindValue(":portable", portable);

    if(!query->exec())
    {
        qDebug() << "Erreur requete SQL modifier personne " << database.lastError() << endl;
        return false;
    }
        return true;
}


///////////SUPPRIMER UNE PERSONNE
bool Bdd::setSuppPersonne(QString num_pers) //Supprimer Personne
    {

    //requete
    requete = "DELETE FROM personne ";
    requete += "WHERE num_pers=:num_pers";
    query->prepare(requete);
    query->bindValue("num_pers", num_pers);

    if(!query->exec()){
        qDebug() << "Erreur requete SQL supp personne " << database.lastError() << endl;
        return false;
    }

    return true;
}











                    ////////////////////////////////////
                    /***            CAMERA          ***/
                    ////////////////////////////////////

///            OBTENIR CAMERAS EXISTANTES            ///
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

///            OBTENIR UN NOUVEAU NUMERO DE CAMERA            ///

int Bdd::getNouveauNumCamera()
{
    requete = "SELECT MAX(Num_Camera)+1 FROM cameras";
    query->prepare(requete);

    if(!query->exec())
    {
        qDebug() << "Erreur requete SQL getNouveauNumCamera " << database.lastError() << endl;
    }

    query->next();

    int num_cam=query->value(0).toInt();

    return num_cam;
}

///            AJOUTER CAMERA            ///
bool Bdd::addCamera(QString Adresse_IP, QString URL, QString Port_TCP, QString Type_Camera, int Num_Camera, QString Num_Lieu, QString Num_Vue)
{
    //requete
    requete = "INSERT INTO cameras (Adresse_IP, URL, Port_TCP, Type_Camera, Num_Camera, Num_Lieu, Num_Vue, estActif)";
    requete += "VALUES (:Adresse_IP, :URL, :Port_TCP, :Type_Camera, :Num_Camera, :Num_Lieu, :Num_Vue, 0)";

    query->prepare(requete);
    query->bindValue(":Adresse_IP", Adresse_IP);
    query->bindValue(":URL", URL);
    query->bindValue(":Port_TCP", Port_TCP);
    query->bindValue(":Type_Camera", Type_Camera);
    query->bindValue(":Num_Camera", Num_Camera);
    query->bindValue(":Num_Lieu", Num_Lieu);
    query->bindValue(":Num_Vue", Num_Vue);

    if(!query->exec()) {
        qDebug() << "Erreur requete SQL ajouter camera" << database.lastError() << endl;
        return false;
    }
    return true;
}

///            MODIFIER CAMERA            ///
bool Bdd::setModCamera(QString Adresse_IP, QString URL, QString Port_TCP, QString Type_Camera, QString selectedNum, QString Num_Lieu, QString Num_Vue)
{
    //obtenir num camera
    int iNum = getNumCamera(Type_Camera);

    //requete
    requete = "UPDATE cameras ";
    requete += "SET Adresse_IP=:Adresse_IP, URL=:URL, Port_TCP=:Port_TCP, Type_Camera=:Type_Camera, Num_Camera=:Num_Camera, Num_Lieu=:Num_Lieu, Num_Vue=:Num_Vue ";
    requete += "WHERE Num_Camera=:selectedNum";

    query->prepare(requete);
    query->bindValue(":selectedNum", selectedNum);
    query->bindValue(":Adresse_IP", Adresse_IP);
    query->bindValue(":URL", URL);
    query->bindValue(":Port_TCP", Port_TCP);
    query->bindValue(":Type_Camera", Type_Camera);
    query->bindValue(":Num_Camera", iNum);
    query->bindValue(":Num_Lieu", Num_Lieu);
    query->bindValue(":Num_Vue", Num_Vue);

    if(!query->exec()) {
        qDebug() << "Erreur requete SQL modif camera" << database.lastError() << endl;
        return false;
    }
    return true;
}

///            SUPPRIMER CAMERA            ///
bool Bdd::setSuppCamera(QString Num_Camera)
{
    //requete
    requete = "DELETE FROM cameras ";
    requete += "WHERE Num_Camera=:Num_Camera";

    query->prepare(requete);
    query->bindValue("Num_Camera", Num_Camera);

    if(!query->exec()) {
        qDebug() << "Erreur requete SQL supprimer camera" << database.lastError() << endl;
        return false;
    }
    return true;
}



///            OBTENIR NUM CAMERA            ///

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




















