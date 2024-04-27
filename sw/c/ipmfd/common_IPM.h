#ifndef __common_h__
#define __common_h__

#include <stdint.h>
extern char msg[50];
typedef struct {
    uint32_t dt_no_counter;
    uint32_t dt_with_counter;
    uint32_t dt_hardware;
} TimeCosts;
extern TimeCosts time_costs;
extern int ipm_mult_count, ipm_square_count, ipm_mask_count, ipm_homo_count, ipm_unmask_count;

typedef unsigned char uint8_t;
void report_cycle();
// void report_time(int dt, int nt, int base, unsigned int nrand);
int check_ciphertext(uint8_t *out, uint8_t *outex, int nbyte);
int runalgo(void (*algo)(const uint8_t *, uint8_t *, const uint8_t *, int, int), uint8_t *in, uint8_t *out, uint8_t *key, int l,
            uint8_t *outex, int nbyte, int nt, int base);
#endif

uint32_t expandToUint32 (uint8_t * X);
void restoreToArray(uint8_t *Y, uint32_t X);
