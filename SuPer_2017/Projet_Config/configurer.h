#ifndef CONFIGURER_H
#define CONFIGURER_H

#include <QMainWindow>
#include "bdd.h"

namespace Ui {
class Configurer;
}

class Configurer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Configurer(Bdd *bdd,QWidget *parent = 0);
    ~Configurer();


    
private slots:
    //slots créer avec QT
    void on_btAffLog_clicked();
    void on_btSuppLog_clicked();

    void on_btAnnulerLier_clicked();
    void on_btAnnulerDelier_clicked();
    void on_cBoxPersonneLier_activated(int index);
    void on_btOkLier_clicked();
    void on_cBoxBadgeSupp_activated(int index);
    void on_btOkDelier_clicked();
    void on_btAffichage_clicked();
    void on_btAnnulerVueAdd_clicked();
    void on_btAnnulerVueMod_clicked();
    void on_btannulerVueSupp_clicked();
    void on_btVueExistante_clicked();
    void on_btOkVueAdd_clicked();
    void on_cBoxVueMod_activated(int index);
    void on_btOkVueMod_clicked();
    void on_cBoxVueSupp_activated(int index);
    void on_btOKVueSupp_clicked();
    void on_btAnnulerLieuAdd_clicked();
    void on_btAnnulerLieuMod_clicked();
    void on_btAnnulerLieuSupp_clicked();
    void on_btLieuExistant_clicked();
    void on_btOkLieuAdd_clicked();
    void on_cBoxLieuMod_activated(int index);
    void on_cBoxLieuSupp_activated(int index);
    void on_btOkLieuMod_clicked();
    void on_btOkLieuSupp_clicked();
    void on_btAnnulerZoneAdd_clicked();
    void on_btAnnulerZoneMod_clicked();
    void on_btAnnulerZoneSupp_clicked();
    void on_btVueExistante_2_clicked();
    void on_btOkZoneAdd_clicked();
    void on_cBoxZoneMod_activated(int index);
    void on_btOkZoneMod_clicked();
    void on_cBoxZoneSupp_activated(int index);
    void on_btOkZoneSupp_clicked();
    void on_cBoxPositionNumVue_activated(int index);
    void on_cBoxPositionNumZone_activated(int index);
    void on_btAnnulerPosition_clicked();
    void on_btOkPosition_clicked();
    void on_cBoxPositionSuppNumVue_activated(int index);
    void on_cBoxPositionSuppNumZone_activated(int index);
    void on_btAnnulerPositionSupp_clicked();
    void on_btOkPositionSupp_clicked();
    void on_btOkTimer_clicked();

    void on_btOkAddLect_clicked();

   // void on_btLecteurExistant_clicked();

    void on_cBoxLectSupp_activated(int index);

    void on_cBoxNumLecteursModif_activated(int index);

    void on_btAnnulerLectAdd_clicked();

    void on_btAnnulerLectModif_clicked();

    void on_btAnnulerLectSupp_clicked();

    void on_btOkLectSupp_clicked();

    void on_btOkLectModif_clicked();

    void on_btOkPersAdd_clicked();

    void on_btAnnulerPersAdd_clicked();

    void on_btOkPersSupp_clicked();

    void on_btAnnulerPersSupp_clicked();

    void on_cBoxPersSupp_activated(int index);

    //void on_cBoxNumLecteursModif_2_activated(int index);

    void on_cBoxPersModif_activated(int index);

    void on_btOkPersMod_clicked();

    void on_btAnnulerPersMod_clicked();



    void on_pbAnnulerCreer_clicked();

    void on_pbConfirmerCreer_clicked();

    void on_cBoxSupp_activated(int index);

    void on_pbAnnulerSupp_clicked();

    void on_pbConfirmSupp_clicked();

    void on_cBoxModif_activated(int index);

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_cBoxAffecterBadge_activated(int index);

    void on_pbConfirmerModifLogin_clicked();

    void on_pbVerificationLogin_clicked();

    void on_lnPassword_returnPressed();

    void on_pbAnnulerCam_clicked();

    void on_pbConfirmerCam_clicked();

    void on_cb_ModifCam_activated(int index);

    void on_pbModifCamAnnuler_clicked();

    void on_pbModifCamConfirmer_clicked();

    void on_cb_SupprCam_activated(int index);

    void on_pbAnnulerSuppr_clicked();

    void on_pbConfirmerSuppr_clicked();

    //slot pour actualiser la date et l'heure
    void sltActualiserDate();

private:
    Ui::Configurer *ui;

    //pointeur sur la classe BDD
    Bdd *pBdd;

    //déclaration QList
    QList<T_Personne> listePersonne, listePersonneALier;
    QList<T_Badge> listeBadge, ListeBadgeLies;
    QList<T_Vue> listeVue;
    QList<T_Lieu> listeLieu;
    QList<T_Zone> listeZone;
    QList<T_Lecteur> listeLecteur;
    QList<T_Representation> listeRepresentation;
    QList<QString> badgeIndisponibles; //contient tout les badges
    QList<T_Camera> listeCamera;

    int etatPosition;  //attribut ajouter (1) ou modifier (0) ou erreur (-1)

    bool actuAffichage();  //méthode permettant l'actualisation de l'affichage

    QTimer *tmrDate; //timer pour actualiser l'heure et la date
};

#endif // CONFIGURER_H
