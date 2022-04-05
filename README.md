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

### SDP

Le serveur expose également un service sdp dont l'UUID est `98592148-f911-4837-9132-ef39f920a5b9`.

Pour faire fonctionner le sdp (résumé de https://raspberrypi.stackexchange.com/a/42262) :
- ajouter ` --compat` à la ligne `ExecStart=/usr/lib/bluetooth/bluetoothd` du fichier `/etc/systemd/system/dbus-org.bluez.service`
- redémarrer le service bluetooth
  ```bash
  sudo systemctl daemon-reload
  sudo systemctl restart bluetooth
  ```

Ensuite on peut lancer le serveur en root (`sudo ./server`), ou bien changer les permissions du ficher `/var/run/sdp` avec
```bash
sudo chmod 666 /var/run/sdp
```
pour pouvoir le lancer normalement. 

## Libs

Les bibliothèques utilisées sont :
- [fmtlib](https://fmt.dev) : pour l'affichage, en remplacement de printf et cout
- [dasynq](https://github.com/davmac314/dasynq) : pour la boucle d'événement, ça permet d'avoir des fonctions appelés de manière asynchrone lorsqu'on reçoit un message sur un socket, ce qui permet de gérer plusieurs connexions à la fois ainsi qu'accepter de nouveaux utilisateurs sur un seul thread
- [protobuf](https://developers.google.com/protocol-buffers/) : pour l'encodage de messages plus complexes que des chaînes de caractères, permet d'encoder / décoder depuis des langages différents

La partie socket et broadcast est basée sur le code de Colin.
