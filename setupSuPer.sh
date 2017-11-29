#!/bin/sh
# Fichier "install"
#############     PRE-REQUIS      ########################
#
#
#   Le fichier de configuration de paquets config_paquet_Linux.xml doit préalablement être importé dans la section Installer ou supprimer des paquets de Yast
#
#
######################CHEMIN D'INSTALLATION################
#récupération du chemin d'install
pathInstall=$PWD"/"
######################TEST CHEMIN EXECUTION ET ADMINISTRATEUR##################
#si le chemin HOME n'est pas celui d'admin l'indiquer et sortir
if [ $HOME != "/root" ]
then 
  echo "Merci de vous connecter en administrateur (su)."
  exit
fi
#si le script n'est pas lancé depuis son répertoire
if [ `dirname $0` != "." ]
then 
  echo "Merci de vous placer dans le répertoire du script avant de le lancer (cd)."
  exit
fi
######################BASE DE DONNEES######################
echo "Installation de la base de données..."
#effacer bdd_super si existante
mysql -uroot -padmin -e "DROP DATABASE IF EXISTS bdd_super2015;"
#créer une bdd vide avec le formatage requis si bdd_super non-existante
mysql -uroot -padmin -e "CREATE DATABASE IF NOT EXISTS bdd_super2015 DEFAULT CHARACTER SET utf8 COLLATE utf8_bin;"
#insérer notre bdd dans la bdd créée précédement
mysql -uroot -padmin  bdd_super2015 < $pathInstall"bdd_SuPer2015.sql"
#créer un user (local) avec privilèges et quelques droits sur la bdd
mysql -uroot -padmin -e "GRANT SELECT,INSERT,DELETE,UPDATE ON bdd_super2015.* TO super2015@localhost identified by 'super2015' WITH GRANT OPTION;"
#créer un user (%) avec privilèges et quelques droits sur la bdd
mysql -uroot -padmin -e "GRANT SELECT,INSERT,DELETE,UPDATE ON bdd_super2015.* TO super2015 identified by 'super2015' WITH GRANT OPTION;"
echo "Base de données installée."
######################INSTALLATION PHP MY ADMIN######################
#tester présence de phpMyAdmin, si oui, supprimer
if [ -d "/srv/www/htdocs/phpMyAdmin/" ]
then
  rm -R /srv/www/htdocs/phpMyAdmin/
  echo "Mise à jour de phpMyAdmin..."
else
  echo "Installation de phpMyAdmin..."
fi
#copier dossier phpMyAdmin dans le serveur www
cp -R $pathInstall"phpMyAdmin/" /srv/www/htdocs/phpMyAdmin/
#copier ficher sample sous autre nom
cp /srv/www/htdocs/phpMyAdmin/config.sample.inc.php /srv/www/htdocs/phpMyAdmin/config.inc.php
####modifier fichier (mode authentification)
sed -i "s/'cookie'/'http'/g" /srv/www/htdocs/phpMyAdmin/config.sample.inc.php
echo "phpMyAdmin installé."
########################ACTIVATION DES SERVICES############################
#programmer start des services mysql et apache2 sur init 3 et 5
chkconfig --level 35 mysql on
chkconfig --level 35 apache2 on
#lancer les services
systemctl start mysql
systemctl start apache2
echo "Services lancés."
########################CONFIGURATION MODULE APACHEphp5############################
#ajout du modules php5 dans le serveur Apache (Network,WWW,Apache2,APACHE_MODULES)
a2enmod php5     # renvoi "php5" already present si module déja ajouté
echo "Module php5 ajouté dans APACHE_MODULES."
########################COPIES LOGICIELLES################################
dossierSuPer="/opt/SuPer_2015/"
#tester présence de SuPer2015, si oui, supprimer
if [ -d $dossierSuPer ]
then
  rm -R $dossierSuPer
  echo "Mise à jour de SuPer 2015..."
else
  echo "Installation de SuPer 2015..."
fi
#copier dossier logiciel 
cp -R $pathInstall"SuPer_2015/" $dossierSuPer
#modifier owner et groupe pour pouvoir ouvrir les fichiers log.txt
chown -R $LOGNAME:$LOGNAME $dossierSuPer
echo "SuPer installé."
#######################COPIES FICHIERS INSTALLATION POUR DEV/MAINTENANCE###########
#tester présence du dossier d'install, si oui, supprimer
if [ -d ""$dossierSuPer"MAINTENANCE(2015)/" ]
then
  rm -R ""$dossierSuPer"MAINTENANCE(2015)/"
  echo "Mise à jour du dossier de maintsenance..."
else
  echo "Installation du dossier de maintenance..."
fi
#copier dossier logiciel 
sudo cp -R $pathInstall ""$dossierSuPer"MAINTENANCE(2015)/"
echo "Dossier de maintenance installé."
######################CREATION DES EXECUTABLES####################################
echo "Création des exécutables..."
#construire les makefile
cd "$dossierSuPer"Projet_Config/
qmake Projet_Config.pro
cd "$dossierSuPer"Projet_SuPer/
qmake Projet_SuPer.pro
#construire les exécutables
cd "$dossierSuPer"Projet_Config/; make
cd "$dossierSuPer"Projet_SuPer/; make
#nettoyer les moc, ui et .o
cd "$dossierSuPer"Projet_Config/; rm moc_* && rm ui_* && rm *.o
cd "$dossierSuPer"Projet_SuPer/; rm moc_* && rm ui_* && rm *.o
if [ -f "$dossierSuPer"Projet_SuPer/Projet_SuPer ] && [ -f "$dossierSuPer"Projet_Config/Projet_Config ]
then
  echo "Exécutables créés."
  #tester présence lien, si oui, les supprimer
  if [ -f "/home/"$USER"/Bureau/SuPer_Configuration.desktop" ]
  then
    rm "/home/"$USER"/Bureau/SuPer_Configuration.desktop"
    echo "Mise à jour des raccourcis..."
  else
    echo "Installation des raccourcis..."
  fi
  #copier le raccourci vers les applications du bureau
  cp $pathInstall"SuPer_Supervision.desktop" "/home/"$USER"/Bureau/SuPer_Supervision.desktop"
  echo "Raccourcis créés."
  #Message final
  echo ""
  echo "L'ensemble du logiciel a été installé."
else 
  echo "Problèmes lors de la création des exécutables."
fi
