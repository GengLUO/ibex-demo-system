#ifndef __AES_IPM_h__
#define __AES_IPM_h__

#include "IPM.h"
typedef unsigned char uint8_t;

void sbox_share(uint8_t *res, const uint8_t *x);

void printMes(const char *s, const uint8_t *m);
void unmaskPrintMes(uint8_t stateshare[16][n], const char *s);

void x127_share(uint8_t *res, const uint8_t *x);
void AES_IPM_FD_Key_Expansion(const uint8_t *key, uint8_t wshare[176][n]);

int AES_IPM_FD(const uint8_t in[16], uint8_t out[16], const uint8_t key[16]);
uint32_t run_aes_share(const uint8_t in[16], uint8_t out[16], const uint8_t key[16], int *status, int dt);
void inv_aes_share(const uint8_t in[16], uint8_t out[16], const uint8_t key[16]);
#endif

uint32_t run_aes_share_hardware(const uint8_t in[16], uint8_t out[16], const uint8_t key[16], int *status, int nt);
