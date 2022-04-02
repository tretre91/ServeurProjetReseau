#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

int create_socket(void);

int init_service(void);

void close_service(void);

#ifdef __cplusplus
}
#endif

#endif // !BLUETOOTH_H
