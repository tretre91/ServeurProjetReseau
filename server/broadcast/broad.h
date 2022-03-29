// glue code pour chacher "broadcast.h" car les compilateurs c++ n'aiment pas les
// types incomplets et les typedefs struct S* S apparement
#ifndef BROAD_H
#define BROAD_H

#ifdef __cplusplus
extern "C" {
#endif

int init_broadcast(const char* ip, int port);

int receive_from_broadcast(char* buffer, int size);

int send_to_broadcast(const char* message, int size);

void close_broadcast();

#ifdef __cplusplus
}
#endif

#endif