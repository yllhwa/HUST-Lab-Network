#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "library.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <time.h>
#include "log.h"

SOCKADDR_IN remote_addr;

void set_remote_ip(char *ip){
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(69);
    remote_addr.sin_addr.S_un.S_addr = inet_addr(ip);
}

struct timeval ACK_TIMEOUT = {3, 0};
struct timeval DATA_TIMEOUT = {3, 0};

// 获取高精度时间(QueryPerformanceFrequency)
double get_time() {
    LARGE_INTEGER time, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (double) time.QuadPart / freq.QuadPart;
}

int get_file(char *filename, char *local_name, int mode) {
    Log(LOG_INFO, "get_file: filename=%s, local_name=%s, mode=%s", filename, local_name, mode == MODE_NETASCII ? "netascii" : "octet");
    // 拷贝remote_addr
    SOCKADDR_IN _remote_addr = remote_addr;
    // 构造请求包
    char *packet;
    char *mode_str = mode == MODE_NETASCII ? "netascii" : "octet";
    int packet_len = 2 + strlen(filename) + 1 + strlen(mode_str) + 1;
    packet = malloc(packet_len);
    sprintf_s(packet, packet_len, "%c%c%s%c%s", 0, OP_RRQ, filename, 0, mode_str);
    // 发送get请求
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sock, packet, packet_len, 0, (SOCKADDR *) &_remote_addr, sizeof(_remote_addr));
    Log(LOG_INFO, "get_file: send RRQ packet");
    free(packet);
    // 等待数据3s超时, select控制超时
    unsigned char buf[516];
    fd_set readfds;
    // 打开文件
    FILE *fp = fopen(local_name, mode==MODE_NETASCII?"w":"wb");
    int block_num = 1;
    int ret_flag = 0;
    // 记录开始时间
    double start_time = get_time();
    int total_bytes = 0;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        int ret = select(0, &readfds, NULL, NULL, &DATA_TIMEOUT);
        if (ret == 0) {
            Log(LOG_WARN, "get_file: wait data timeout");
            ret_flag = -1;
            break;
        }
        if (ret == SOCKET_ERROR) {
            Log(LOG_ERROR, "get_file: socket error");
            ret_flag = -1;
            break;
        }
        if (FD_ISSET(sock, &readfds)) {
            int len = sizeof(remote_addr);
            int recv_len = recvfrom(sock, buf, 516, 0, (SOCKADDR *) &_remote_addr, &len);
            if (recv_len == SOCKET_ERROR) {
                Log(LOG_ERROR, "get_file: recv data error");
                ret_flag = -1;
                break;
            }
            if (buf[1] == OP_ERROR) {
                // 记录error message
                Log(LOG_ERROR, "get_file: server error, code: %d, message: %s", buf[4], buf + 4);
                ret_flag = -1;
                break;
            } else if (buf[1] == OP_DATA) {
                int recv_block_num = (buf[2] << 8) + buf[3];
                Log(LOG_INFO, "get_file: recv data block_num=%d", recv_block_num);
                if (recv_block_num == block_num) {
                    // 发送ACK
                    char ack[4];
                    ack[0] = 0;
                    ack[1] = OP_ACK;
                    ack[2] = buf[2];
                    ack[3] = buf[3];
                    sendto(sock, ack, 4, 0, (SOCKADDR *) &_remote_addr, sizeof(_remote_addr));
                    Log(LOG_INFO, "get_file: send ACK, block_num: %d", block_num);
                    // 写入文件
                    fwrite(buf + 4, 1, recv_len - 4, fp);
                    // 判断是否结束
                    if (recv_len < 516) {
                        Log(LOG_INFO, "get_file: recv data end");
                        ret_flag = 0;
                        break;
                    }
                    // 记录速度
                    total_bytes += recv_len - 4;
                    double now_time = get_time();
                    double time_diff = now_time - start_time;
                    // 考虑end_time-start_time=0的情况
                    if (time_diff > 0) {
                        Log(LOG_INFO, "get_file: speed: %.2f KB/s", total_bytes / time_diff / 1024);
                        start_time = now_time;
                        total_bytes = 0;
                    }
                    block_num++;
                } else {
                    // 重发上一条的ack
                    char ack[4];
                    ack[0] = 0;
                    ack[1] = OP_ACK;
                    ack[2] = buf[2];
                    ack[3] = buf[3];
                    sendto(sock, ack, 4, 0, (SOCKADDR *) &_remote_addr, sizeof(_remote_addr));
                    Log(LOG_INFO, "get_file: resend ACK, block_num: %d", block_num);
                }
            }
        }
    }
    Log(LOG_INFO, "get_file: finished");
    fclose(fp);
    closesocket(sock);
    return ret_flag;
}

int wait_ack(SOCKET sock, int block_num, SOCKADDR_IN* _remote_addr) {
    char buf[516];
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    int ret = select(0, &readfds, NULL, NULL, &ACK_TIMEOUT);
    if (ret == 0) {
        Log(LOG_WARN, "wait_ack: wait ack timeout");
        return -2;
    }
    if (ret == SOCKET_ERROR) {
        Log(LOG_ERROR, "wait_ack: socket error");
        return -1;
    }
    if (FD_ISSET(sock, &readfds)) {
        int len = sizeof(*_remote_addr);
        int recv_len = recvfrom(sock, buf, 516, 0, _remote_addr, &len);
        if (recv_len == SOCKET_ERROR) {
            Log(LOG_ERROR, "wait_ack: recv ack error");
            return -1;
        }
        if (buf[1] == OP_ERROR) {
            // 记录error message
            Log(LOG_ERROR, "wait_ack: server error, code: %d, message: %s", buf[4], buf + 4);
            return -3;
        }
        if (buf[1] == OP_ACK) {
            int recv_block_num = (buf[2] << 8) + buf[3];
            if (recv_block_num == block_num) {
                Log(LOG_INFO, "wait_ack: recv ACK, block_num: %d", block_num);
                return 0;
            }
        }
    }
    return -4;
}

int put_file(char *filename, char *local_name, int mode) {
    Log(LOG_INFO, "put_file: filename: %s, local_name: %s, mode: %d", filename, local_name, mode);
    // 拷贝remote_addr
    SOCKADDR_IN _remote_addr = remote_addr;
    // 构造请求包
    char *packet;
    char *mode_str = mode == MODE_NETASCII ? "netascii" : "octet";
    int packet_len = 2 + strlen(filename) + 1 + strlen(mode_str) + 1;
    packet = malloc(packet_len);
    sprintf_s(packet, packet_len, "%c%c%s%c%s", 0, OP_WRQ, filename, 0, mode_str);
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sock, packet, packet_len, 0, (SOCKADDR *) &_remote_addr, sizeof(_remote_addr));
    Log(LOG_INFO, "put_file: send WRQ, filename: %s", filename);
    free(packet);
    // 等待ack
    int ret = wait_ack(sock, 0, &_remote_addr);
    if (ret != 0) {
        Log(LOG_ERROR, "put_file: wait ack error, ret: %d", ret);
        return -1;
    }
    // 打开文件
    FILE *fp = fopen(local_name, mode==MODE_NETASCII?"r":"rb");
    int block_num = 1;
    int ret_flag = 0;
    while (1) {
        // 读取文件
        char buf[512];
        int len = fread(buf, 1, 512, fp);
        if (len == 0) {
            Log(LOG_INFO, "put_file: finished");
            ret_flag = 0;
            break;
        }
        // 构造数据包
        char *packet;
        int packet_len = 4 + len;
        packet = malloc(packet_len);
        packet[0] = 0;
        packet[1] = OP_DATA;
        packet[2] = block_num >> 8;
        packet[3] = block_num & 0xff;
        memcpy(packet + 4, buf, len);
        // 发送数据包
        sendto(sock, packet, packet_len, 0, (SOCKADDR *) &_remote_addr, sizeof(_remote_addr));
        Log(LOG_INFO, "put_file: send DATA, block_num: %d", block_num);
        free(packet);
        // 等待ack
        ret = wait_ack(sock, block_num, &_remote_addr);
        if (ret != 0) {
            Log(LOG_ERROR, "put_file: wait ack error, ret: %d", ret);
            // 文件指针回退
            fseek(fp, -len, SEEK_CUR);
            continue;
        } else {
            block_num++;
        }
    }
    fclose(fp);
    closesocket(sock);
    return ret_flag;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // 初始化WSA
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            // 初始化log
            init_log();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}