#include "bluetooth.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <sys/socket.h>
#include <unistd.h>

// objet représentant la session en cours avec le serveur SDP
sdp_session_t* session = NULL;

int create_socket(void) {
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        return -1;
    }

    struct sockaddr_rc local_address = {0};
    local_address.rc_family = AF_BLUETOOTH;
    bacpy(&local_address.rc_bdaddr, BDADDR_ANY);
    local_address.rc_channel = 1;

    if (bind(sock, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
        perror("Echec de bind : ");
        close(sock);
        return -1;
    }

    if (listen(sock, 10) == -1) {
        perror("Echec de listen : ");
        close(sock);
        return -1;
    }

    return sock;
}

int init_service(void) {
    // uuid : 98592148-f911-4837-9132-ef39f920a5b9
    uint8_t svc_uuid_int[16] = {0x98, 0x59, 0x21, 0x48, 0xF9, 0x11, 0x48, 0x37, 0x91, 0x32, 0xEF, 0x39, 0xF9, 0x20, 0xA5, 0xB9};
    uint8_t rfcomm_channel = 1;
    const char* service_name = "ServeurProjetReseau";
    const char* service_dsc = "Un serveur pour le projet de reseau";
    const char* service_prov = "tretre91";

    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid;

    sdp_record_t record = {0};

    // id de service
    sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
    sdp_set_service_id(&record, svc_uuid);

    // classe du service
    sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
    sdp_list_t* svc_class_list = sdp_list_append(0, &svc_class_uuid);
    sdp_set_service_classes(&record, svc_class_list);

    // profile bluetooth
    sdp_profile_desc_t profile;
    sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
    profile.version = 0x0100;
    sdp_list_t* profile_list = sdp_list_append(0, &profile);
    sdp_set_profile_descs(&record, profile_list);

    // visibilite du service
    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
    sdp_list_t* root_list = sdp_list_append(0, &root_uuid);
    sdp_set_browse_groups(&record, root_list);

    // infos l2cap
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    sdp_list_t* l2cap_list = sdp_list_append(0, &l2cap_uuid);
    sdp_list_t* proto_list = sdp_list_append(0, l2cap_list);

    // canal RFCOMM
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    sdp_data_t* channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
    sdp_list_t* rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    sdp_list_append(rfcomm_list, channel);
    sdp_list_append(proto_list, rfcomm_list);
    sdp_list_t* access_proto_list = sdp_list_append(0, proto_list);
    sdp_set_access_protos(&record, access_proto_list);

    // nom du service, auteur et description
    sdp_set_info_attr(&record, service_name, service_prov, service_dsc);

    // connexion au serveur sdp
    session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
    if (session != NULL) {
        sdp_record_register(session, &record, 0);
    }

    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);

    return session != NULL;
}

void close_service(void) {
    if (session != NULL) {
        sdp_close(session);
        session = NULL;
    }
}
