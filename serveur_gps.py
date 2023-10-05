# -*- coding: utf-8 -*-

# Ce script permet de lire tout un ensemble de trames NMEA stockées dans un fichier texte
# pour les envoyer une par une à des clients TCP.
# Cela permet de simuler la connexion à un GPS en TCP au lieu de RS232

# Les bibliotheques utilisee
import sys
import os
import socket
import threading
import signal
import string
import time


# Variable partagee par les 2 threads
laTramePartagee = "..."


## Classe pour gerer la lecture des trames NMEA dans le fichier texte
#  ...
class ThreadFichier(threading.Thread) :

    ## Methode constructeur
    #  @param self Un pointeur vers l'objet lui-meme
    def __init__(self, nom_fichier) :

        # Lecture du fichier contenant les trames NMEA
        print("Thread FICHIER    => Ouverture de ", nom_fichier)
        self.fichier_nmea = open(nom_fichier, "r")
        self.lignes  = self.fichier_nmea.readlines()
        self.fichier_nmea.close()

        # Modification des trames pour insérer le CR avant LF
        #print("Thread FICHIER    => Modification du CR+LF")
        #for ligne in self.lignes:
        #    resultat = ligne.replace("\n", "\r\n")
        #    ligne = resultat


        """
        print("Thread FICHIER    => ", self.lignes[0])
        print("Thread FICHIER    => ", self.lignes[1])
        print("Thread FICHIER    => ", self.lignes[2])
        """

        # Initialisation de la thread
        threading.Thread.__init__ (self)


    ## Methode surchargee pour choisir toutes les secondes une nouvelle trame à envoyer
    #  @param self Un pointeur vers l'objet lui-meme
    def run(self) :

        # Declaration de la variable globale partagee
        global laTramePartagee

        # Boucle de lecture
        for ligne in self.lignes:
            laTramePartagee = ligne
            #print("Thread FICHIER => ", ligne)
            time.sleep(1)


## Classe pour gerer les clients TCP
#  ...
class ThreadReseau(threading.Thread) :

    ## Methode constructeur
    #  @param self Un pointeur vers l'objet lui-meme
    def __init__(self, channel, details) :

        # Ouverture du port RS232
        self.channel = channel
        self.details = details
        threading.Thread.__init__ (self)


    ## Methode surchargee pour répondre aux clients TCP
    #  @param self Un pointeur vers l'objet lui-meme
    def run(self) :

        # Declaration de la variable globale partagee
        global laTramePartagee

        # Affichage des details de connexion
        print("Thread RESEAU     => Nouvelle connexion : ", self.details[0])

        # Boucle de lecture et d'envoi
        while True:
            try :
                reception = self.channel.recv(1024)
                print("Thread RESEAU     => Reçu de", self.details[0], ": ", reception, len(reception) )
                if (	len(reception) == 6 and
						reception[0] == 0x52 and
						reception[1] == 0x45 and
						reception[2] == 0x54 and
						reception[3] == 0x52 and
						reception[4] == 0x0d and
						reception[5] == 0x0a) :
                    self.channel.send(bytes(laTramePartagee, 'UTF-8'))
                else :
                    self.channel.send(bytes("?\r\n", 'UTF-8'))
            except :
                # Fermeture de la connexion
                self.channel.close()
                print("Thread RESEAU     => Déconnexion de", self.details[0])
                break



# Point d'entree du script
print("**********************")
print("** Serveur GPS v0.1 **")
print("**********************")

# Paramètres de l'application
fichier_nmea = "marathon_la_rochelle_30112008.nmea.ter.txt"
port_tcp = 1664
fin = False

# Pour sortir normalement proprement avec un CTRL+C
signal.signal(signal.SIGINT, signal.default_int_handler)

try :
    # Lancement de la thread FICHIER
    ThreadFichier(fichier_nmea).start()

    # Creation de la socket serveur
    socketServeur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Attachement de la socket
    socketServeur.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    socketServeur.bind(("", port_tcp))

    # Creation d'une file d'attente de clients
    socketServeur.listen(15)

    # Boucle d'ecoute
    print("Thread PRINCIPALE => Serveur en écoute sur ", port_tcp)
    while (fin == False) :
        channel, details = socketServeur.accept()
        ThreadReseau(channel, details).start()

except KeyboardInterrupt :
    print("Au revoir ...")
    fin = True
    socketServeur.close()
