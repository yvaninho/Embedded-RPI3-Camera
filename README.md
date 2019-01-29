# Embedded-RPI3-Camera
Le but du projet est de contrôler une caméra via une RPI3. Pour cela nous avons 3 composants principaux à savoir: Un client, et 2 serveurs donc 1 pour la caméra et l'autre pour le servomoteur. 


#-------------------------1-composant caméra---------------------------
------------------------------------------------------------------------

#a) Tout d'abord il est important de construire notre OS avec le buidroot et la chaine de cross compilation. 
Pour cela: 
|
#	1-Récupéront l'image docker suivante:

$ docker pull pblottiere/embsys-rpi3-buildroot-video

#	2- Créer un conteneur 

$ docker run -it pblottiere/embsys-rpi3-buildroot-video /bin/bash
docker# cd /root
docker# ls
buildroot-precompiled-2017.08.tar.gz
docker# tar zxvf buildroot-precompiled-2017.08.tar.gz

#	3- Flasher la rasberry
  

	(A compléter)


#	4-Cross compiler 

Pour obtenir le fichier cross-compilé on fait comme suite:
	On va tout d'abord s'appuyer sur le travail fait ici :
	https://github.com/twam/v4l2grab/wiki/Installation
suivre la procédure jusqu'au N°5. et la gaire pluôt.

./configure CC=/root/buildroot-precompiled-2017.08/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-gcc --host=linux

puis suivre l'installation avec Run make tapez la commande 
	make

# On obtient ainsi un fichier exécutable pour la machine cible. Ceci permettant de prndre une photo ou une vidéo à partir de la rasberry Pi
                  
