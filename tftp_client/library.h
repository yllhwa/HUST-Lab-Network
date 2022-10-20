#ifndef UNTITLED4_LIBRARY_H
#define UNTITLED4_LIBRARY_H
#define export __declspec(dllexport)

#define MODE_NETASCII 0
#define MODE_OCTET 1

#define OP_RRQ 1
#define OP_WRQ 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

export void set_remote_ip(char *ip);
export int get_file(char *filename, char *local_name, int mode);
export int put_file(char *filename, char *local_name, int mode);

#endif //UNTITLED4_LIBRARY_H
