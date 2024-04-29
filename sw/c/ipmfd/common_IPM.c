// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published
// by the Free Software Foundation.

#include "common_IPM.h"
#include "IPM.h"
#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
#include <string.h>
#include <stddef.h>

char msg[50];
TimeCosts time_costs;
int ipm_mult_count, ipm_square_count, ipm_mask_count, ipm_homo_count, ipm_unmask_count;

void report_cycle(){
  sprintf(msg, "Time cost without countermeasure:\t\t%lu\n", time_costs.dt_no_counter);
  puts(msg);
  sprintf(msg, "Time cost with countermeasure in software:\t%lu\n", time_costs.dt_with_counter);
  puts(msg);
  sprintf(msg, "Time cost with countermeasure in hardware:\t%lu\n", time_costs.dt_hardware);
  puts(msg);

  uint32_t diff = time_costs.dt_with_counter - time_costs.dt_hardware;
  sprintf(msg, "Time saved after using hardware:\t\t%lu\n", diff);
  puts(msg);

  sprintf(msg, "Number of IPM_Mult:\t%d\n", ipm_mult_count);
  puts(msg);
  sprintf(msg, "Number of IPM_Square:\t%d\n", ipm_square_count);
  puts(msg);
  sprintf(msg, "Number of IPM_Mask:\t%d\n", ipm_mask_count);
  puts(msg);
  sprintf(msg, "Number of IPM_Homo:\t%d\n", ipm_homo_count);
  puts(msg);
}

// void report_time(int dt, int nt, int base, unsigned int nrand)
// {
  
//   if (base == 0)
//     base = dt;
//   sprintf(msg, "time = %f ms | pen = %d | rand = %d\n", 1000 * ((float)dt) / CLOCKS_PER_SEC / nt, dt / base, nrand / nt);
//   puts(msg);
//   //sprintf(msg, "time = %f clocks | pen = %d | rand = %d\n", ((float)dt) / nt, dt / base, nrand / nt);
// }

int check_ciphertext(uint8_t *out, uint8_t *outex, int nbyte)
{
  int error;
  if (memcmp(out, outex, nbyte) != 0)
  {
    error = 1;
    puts("**ERROR: incorrect ciphertext\n");
    //exit(EXIT_FAILURE);
  }
  else {
    error = 0;
    puts("SUCCESS: correct ciphertext !\n");
  }
  return error;
}

int runalgo(void (*algo)(const uint8_t *, uint8_t *, const uint8_t *, int, int), uint8_t *in, uint8_t *out, uint8_t *key, int l,
            uint8_t *outex, int nbyte, int nt, int base)
{
  int i;
  uint32_t start, end;

  asm volatile ("csrr %0, mcycle" : "=r" (start));

  for (i = 0; i < nt; i++)
    algo(in, out, key, 4, l);
  asm volatile ("csrr %0, mcycle" : "=r" (end));
  check_ciphertext(out, outex, nbyte);
  return (int)(end - start);
}

uint32_t expandToUint32 (uint8_t * X){
    return ((uint32_t)X[3] | ((uint32_t)X[2] << 8) | ((uint32_t)X[1] << 16) | ((uint32_t)X[0] << 24));
}

void restoreToArray(uint8_t *Y, uint32_t X){
    Y[3] = (uint8_t)X;
    Y[2] = (uint8_t)(X >> 8);
    Y[1] = (uint8_t)(X >> 16);
    Y[0] = (uint8_t)(X >> 24);
}