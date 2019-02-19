# Embedded-RPI3-Camera

Le but du projet est de contrôler une caméra via une RPI3. Pour cela nous avons 3 composants principaux à savoir: Un client qui , et 2 serveurs donc 1 pour la caméra et l'autre pour le cerveau moteur  


Lancer mon serveurMoteur
 - Lancer le fichier serveurMoteur.py
 - le serveur est connecté sur le port 5000
 - Passer des arguments du type a,b,c où a,b,c sont des entiers a: sensRotation b= angleRotation c= tempsRotation 
 si a=1 rotation dans le sens des aiguilles d'une montre
si a=2 sens contraire des aiguilles d'une montre
