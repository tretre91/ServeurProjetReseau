# Projet de Réseau

Serveur bluetooth pour le projet de Réseaux avancés

Pour les clients :
- https://github.com/Juroupi/AppProjetReseau (app mobile)
- https://github.com/tretre91/ClientProjetReseau (ligne de commande)

## Build

Pour compiler vous aurez besoin de cmake, pour l'installer vous pouvez simplement faire `sudo apt install cmake`.
On a aussi besoin de :
- libbluetooth-dev (`sudo apt install libbluetooth-dev`)
- protobuf (`sudo apt install protobuf-compiler`)
- une connexion à internet (seulement lors de la première configuration du projet).

```bash
$ git clone https://gitlab.dsi.universite-paris-saclay.fr/trevis.morvany/projetreseau
$ cd projetreseau
$ cmake -S . -B build -G "Unix Makefiles"
$ cd build
$ make server 
```

## Utilisation

Le serveur gère plusieurs connections à la fois, il affiche dans la console les messages reçus et les renvoie à tous les clients connectés.

**Remarque :** pour que le pi puisse accepter des connexions il doit être visible en bluetooth (`bluetoothctl discoverable on`).

pour le lancer, simplement faire `./server`.

Commandes disponibles :
- `stop` : arrête le serveur, il envoie un message "stop" à tous ses clients avant de s'arrêter
- `stat` : affiche des statistiques sur le serveur

## Libs

Les bibliothèques utilisées sont :
- [fmtlib](https://fmt.dev) : pour l'affichage, en remplacement de printf et cout
- [dasynq](https://github.com/davmac314/dasynq) : pour la boucle d'événement, ça permet d'avoir des fonctions appelés de manière asynchrone lorsqu'on reçoit un message sur un socket, ce qui permet de gérer plusieurs connexions à la fois ainsi qu'accepter de nouveaux utilisateurs sur un seul thread
- [protobuf](https://developers.google.com/protocol-buffers/) : pour l'encodage de messages plus complexes que des chaînes de caractères, permet d'encoder / décoder depuis des langages différents

La partie socket et broadcast est basée sur le code de Colin.

## Tâches à réaliser

voir https://hackmd.io/@pOKjsj05QGK4BisC1CfjbQ/ByJgKU0Z9
