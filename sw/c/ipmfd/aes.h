// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

#ifndef __aes_h__
#define __aes_h__

#include "common_IPM.h"

extern uint8_t sbox[256];

uint8_t multx(uint8_t x);
uint8_t GF256_Inverse(uint8_t x);

uint8_t bit(uint8_t x, int i);
uint8_t affine(uint8_t x);

uint8_t subbyte(uint8_t x);
void printstate(uint8_t state[16]);

void shiftrows(uint8_t state[16]);
void mixcolumns(uint8_t *state);
void subbytestate(uint8_t *state);
void addroundkey(uint8_t *state, uint8_t *w, int round);

void keyexpansion(uint8_t *key, uint8_t *w);

void aes(uint8_t in[16], uint8_t out[16], uint8_t *key);
uint32_t run_aes(uint8_t in[16], uint8_t out[16], uint8_t *key, int nt);

void testaes();

#endif
