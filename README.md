# Projet de Réseau

## Tâches à réaliser

voir https://hackmd.io/@pOKjsj05QGK4BisC1CfjbQ/ByJgKU0Z9

## Build

Pour compiler les différents exécutables vous aurez besoin de cmake, pour l'installer vous pouvez simplement faire `sudo apt install cmake`.
On a aussi besoin de libbluetooth-dev (`sudo apt install libbluetooth-dev`).

```bash
$ git clone https://gitlab.dsi.universite-paris-saclay.fr/trevis.morvany/projetreseau
$ cd projetreseau
$ cmake -S . -B build -G "Unix Makefiles"
$ cd build
$ make server # ou 'make client' pour le client
```

un fois compilé le client se trouve dans le dossier client/ et le serveur dans le dossier serveur/

## Client

Un simple client en ligne de commande qui envoie au serveur les lignes de texte lues sur l'entrée standard et écrit les messages reçus par le serveur.

Pour l'exécuter (depuis le dossier build) : `./client/client <adresse>` avec `adresse` l'adresse bluetooth du raspberry pi (ou d'une autre machine qui fait tourner le serveur).

## Serveur

Le serveur gère plusieurs connections à la fois, il affiche dans la console les messages reçus et les renvoie à tous les clients connectés.

Pour que le pi puisse accepter des connexions il doit être visible en bluetooth (`bluetoothctl discoverable on`).
 