#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Crée un socket bluetooth qui écoute les connexions entrantes
 * @return le descripteur du socket
 */
int create_socket(void);

/**
 * @brief Ajoute le service de messagerie au serveur SDP local
 * @return 1 (true) si l'ajout a réussi, ou 0 (false) en cas d'erreur
 */
int init_service(void);

/**
 * @brief Retire le service de messagerie du serveur SDP
 */
void close_service(void);

#ifdef __cplusplus
}
#endif

#endif // !BLUETOOTH_H
