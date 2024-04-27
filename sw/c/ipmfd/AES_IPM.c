//secure AES implemention with IPM faults detection

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <time.h>

#include "aes.h"
#include "IPM.h"
#include "AES_IPM.h"
#include "timer.h"
#include "demo_system.h"
#include "dev_access.h"

#define CIPHER_VERBOSE 0
#define DECIPHER_VERBOSE 0


//raise sharing to the power 127
void x127_share(uint8_t *res, const uint8_t *x)
{
	// use 6 squarings and 4 multiplications, can we do better?
	uint8_t z[n];
	IPM_FD_Square(z, x); // z=x^2

	uint8_t y[n];
	IPM_FD_Mult(y, x, z); // y=z*x=x^3

	uint8_t w[n];
	IPM_FD_Square(w, y); //6

	IPM_FD_Mult(z, w, x); // z =x^7
	IPM_FD_Square(w, w);  // w=x^12

	uint8_t y2[n];
	IPM_FD_Mult(y2, w, y); // y2=x^15

	IPM_FD_Square(y2, y2); //30
	IPM_FD_Square(y2, y2); //60
	IPM_FD_Square(y2, y2); //120

	IPM_FD_Mult(res, y2, z); // y=x^127
}
void x127_share_hardware(uint8_t *res, const uint8_t *x)
{
	// use 6 squarings and 4 multiplications, can we do better?
	uint8_t z[n];
	IPM_FD_Square_hardware(z, x); // z=x^2
	// print(z, "z:");
	uint8_t y[n];
	IPM_FD_Mult_hardware(y, x, z); // y=z*x=x^3

	uint8_t w[n];
	IPM_FD_Square_hardware(w, y); //6

	IPM_FD_Mult_hardware(z, w, x); // z =x^7
	IPM_FD_Square_hardware(w, w);  // w=x^12

	uint8_t y2[n];
	IPM_FD_Mult_hardware(y2, w, y); // y2=x^15

	IPM_FD_Square_hardware(y2, y2); //30
	IPM_FD_Square_hardware(y2, y2); //60
	IPM_FD_Square_hardware(y2, y2); //120

	IPM_FD_Mult_hardware(res, y2, z); // y=x^127
}

//perform the sbox transformation on masked vector
//using sbox polynomial expression
//63 + 8fx^127 + b5x^191 + 01x^223 + f4x^239 + 25x^247 + f9x^251 + 09x^253 + 05x^254
void sbox_share(uint8_t *res, const uint8_t *x)
{
	uint8_t x254[n], x253[n], x251[n], x247[n], x239[n], x223[n], x191[n], x127[n];

	x127_share(x127, x);
	IPM_FD_Square(x254, x127);

	IPM_FD_Square(x253, x254);
	IPM_FD_Square(x251, x253); //251 = (253* 2)-255
	IPM_FD_Square(x247, x251);

	IPM_FD_Square(x239, x247);
	IPM_FD_Square(x223, x239); //x223 = (x239)^2
	IPM_FD_Square(x191, x223); //x^191 = (x^223)^2

	IPConstMult(x254, x254, 0x05);
	IPConstMult(x253, x253, 0x09);
	IPConstMult(x251, x251, 0xf9);

	IPConstMult(x247, x247, 0x25);
	IPConstMult(x239, x239, 0xf4);
	//IPConstMult(x223, x223,0x01);
	IPConstMult(x191, x191, 0xb5);
	IPConstMult(x127, x127, 0x8f);

	IPConstAdd(res, x127, 0x63);
	for (int i = 0; i < n; i++)
		res[i] ^= x191[i] ^ x223[i] ^ x239[i] ^ x247[i] ^ x251[i] ^ x253[i] ^ x254[i];
}
void sbox_share_hardware(uint8_t *res, const uint8_t *x)
{
	uint8_t x254[n], x253[n], x251[n], x247[n], x239[n], x223[n], x191[n], x127[n];
	x127_share_hardware(x127, x);

	IPM_FD_Square_hardware(x254, x127);

	IPM_FD_Square_hardware(x253, x254);
	IPM_FD_Square_hardware(x251, x253); //251 = (253* 2)-255
	IPM_FD_Square_hardware(x247, x251);

	IPM_FD_Square_hardware(x239, x247);
	IPM_FD_Square_hardware(x223, x239); //x223 = (x239)^2
	IPM_FD_Square_hardware(x191, x223); //x^191 = (x^223)^2

	IPConstMult(x254, x254, 0x05);
	IPConstMult(x253, x253, 0x09);
	IPConstMult(x251, x251, 0xf9);

	IPConstMult(x247, x247, 0x25);
	IPConstMult(x239, x239, 0xf4);
	//IPConstMult(x223, x223,0x01);
	IPConstMult(x191, x191, 0xb5);
	IPConstMult(x127, x127, 0x8f);

	IPConstAdd(res, x127, 0x63);
	for (int i = 0; i < n; i++)
		res[i] ^= x191[i] ^ x223[i] ^ x239[i] ^ x247[i] ^ x251[i] ^ x253[i] ^ x254[i];
}

//subbyte operation on masked state
void subbyte_share(uint8_t stateshare[16][n])
{
	int j;
	for (j = 0; j < 16; j++)
		sbox_share(stateshare[j], stateshare[j]);
}
void subbyte_share_hardware(uint8_t stateshare[16][n])
{
	int j;
	for (j = 0; j < 16; j++)
		sbox_share_hardware(stateshare[j], stateshare[j]);
}

//TODO complete
void inv_subbyte_share(uint8_t stateshare[16][n])
{
	for (int j = 0; j < 16; j++)
	{
		//find the inv S-box polynomial form
	}
}

void shiftrows_share(uint8_t stateshare[16][n])
{
	uint8_t m;
	int i;
	for (i = 0; i < n; i++)
	{
		m = stateshare[1][i];
		stateshare[1][i] = stateshare[5][i];
		stateshare[5][i] = stateshare[9][i];
		stateshare[9][i] = stateshare[13][i];
		stateshare[13][i] = m;

		m = stateshare[2][i];
		stateshare[2][i] = stateshare[10][i];
		stateshare[10][i] = m;
		m = stateshare[6][i];
		stateshare[6][i] = stateshare[14][i];
		stateshare[14][i] = m;

		m = stateshare[3][i];
		stateshare[3][i] = stateshare[15][i];
		stateshare[15][i] = stateshare[11][i];
		stateshare[11][i] = stateshare[7][i];
		stateshare[7][i] = m;
	}
}

void inv_shiftrows_share(uint8_t stateshare[16][n])
{
	uint8_t m;
	int i;
	for (i = 0; i < n; i++)
	{
		m = stateshare[13][i];

		stateshare[13][i] = stateshare[9][i];
		stateshare[9][i] = stateshare[5][i];
		stateshare[5][i] = stateshare[1][i];
		stateshare[1][i] = m;

		m = stateshare[2][i];
		stateshare[2][i] = stateshare[10][i];
		stateshare[10][i] = m;
		m = stateshare[6][i];
		stateshare[6][i] = stateshare[14][i];
		stateshare[14][i] = m;

		m = stateshare[15][i];
		stateshare[15][i] = stateshare[3][i];
		stateshare[3][i] = stateshare[7][i];
		stateshare[7][i] = stateshare[11][i];
		stateshare[11][i] = m;
	}
}

void mixcolumns_share(uint8_t stateshare[16][n])
{
	uint8_t ns[16];
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < 4; j++)
		{
			ns[j * 4] = multx(stateshare[j * 4][i]) ^ multx(stateshare[j * 4 + 1][i]) ^ stateshare[j * 4 + 1][i] ^ stateshare[j * 4 + 2][i] ^ stateshare[j * 4 + 3][i];
			ns[j * 4 + 1] = stateshare[j * 4][i] ^ multx(stateshare[j * 4 + 1][i]) ^ multx(stateshare[j * 4 + 2][i]) ^ stateshare[j * 4 + 2][i] ^ stateshare[j * 4 + 3][i];
			ns[j * 4 + 2] = stateshare[j * 4][i] ^ stateshare[j * 4 + 1][i] ^ multx(stateshare[j * 4 + 2][i]) ^ multx(stateshare[j * 4 + 3][i]) ^ stateshare[j * 4 + 3][i];
			ns[j * 4 + 3] = multx(stateshare[j * 4][i]) ^ stateshare[j * 4][i] ^ stateshare[j * 4 + 1][i] ^ stateshare[j * 4 + 2][i] ^ multx(stateshare[j * 4 + 3][i]);
		}
		for (j = 0; j < 16; j++)
			stateshare[j][i] = ns[j];
	}
}

void inv_mixcolumns_share(uint8_t stateshare[16][n])
{
	uint8_t ns[16];
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < 4; j++)
		{
			ns[j * 4 + 0] = GF256_Mult(stateshare[j * 4][i], 0x0e) ^ GF256_Mult(stateshare[j * 4 + 1][i], 0x0b) ^ GF256_Mult(stateshare[j * 4 + 2][i], 0x0d) ^ GF256_Mult(stateshare[j * 4 + 3][i], 0x09);
			ns[j * 4 + 1] = GF256_Mult(stateshare[j * 4][i], 0x09) ^ GF256_Mult(stateshare[j * 4 + 1][i], 0x0e) ^ GF256_Mult(stateshare[j * 4 + 2][i], 0x0b) ^ GF256_Mult(stateshare[j * 4 + 3][i], 0x0d);
			ns[j * 4 + 2] = GF256_Mult(stateshare[j * 4][i], 0x0d) ^ GF256_Mult(stateshare[j * 4 + 1][i], 0x09) ^ GF256_Mult(stateshare[j * 4 + 2][i], 0x0e) ^ GF256_Mult(stateshare[j * 4 + 3][i], 0x0b);
			ns[j * 4 + 3] = GF256_Mult(stateshare[j * 4][i], 0x0b) ^ GF256_Mult(stateshare[j * 4 + 1][i], 0x0d) ^ GF256_Mult(stateshare[j * 4 + 2][i], 0x09) ^ GF256_Mult(stateshare[j * 4 + 3][i], 0x0e);
		}
		for (j = 0; j < 16; j++)
			stateshare[j][i] = ns[j];
	}
}

void addroundkey_share(uint8_t stateshare[16][n], uint8_t wshare[176][n], int round)
{
	int i, j;
	for (i = 0; i < 16; i++)
		for (j = 0; j < n; j++)
			stateshare[i][j] ^= wshare[16 * round + i][j];
}

int inv_aes_share_subkeys(const uint8_t in[16], uint8_t out[16], uint8_t wshare[176][n])
{
	int i,j;
	int round = 10;
	uint8_t stateshare[16][n];

	for (i = 0; i < 16; i++)
	{
		//stateshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t));
		mask(stateshare[i], in[i]);
	}

	if (DECIPHER_VERBOSE)
	{
		sprintf(msg, "\n**round %d\n", round);
		puts(msg);
		printMes("input", in);
	}
	addroundkey_share(stateshare, wshare, 10);

	if (DECIPHER_VERBOSE)
		unmaskPrintMes(stateshare, "add round key");

	for (round = 9; round >= 1; round--)
	{
		if (DECIPHER_VERBOSE){
			sprintf(msg, "\n**round %d\n", round);
			puts(msg);
		}

		inv_shiftrows_share(stateshare);
		if (DECIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "inv shift rows");

		inv_subbyte_share(stateshare);
		if (DECIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "inv subbyte");

		addroundkey_share(stateshare, wshare, round);
		if (DECIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "add round key");

		inv_mixcolumns_share(stateshare);
		if (DECIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "inv mix columns");
	}

	inv_shiftrows_share(stateshare);
	inv_subbyte_share(stateshare);
	addroundkey_share(stateshare, wshare, 0);

	int status = 1; //determine if computaion is correct (not faulted)

	for (i = 0; i < 16; i++)
	{ //check for faults before proceeding

		int unmasked = unmask(stateshare[i]);
		if (unmasked != NULL_BYTE) //TODO : leakage !!
			out[i] = (uint8_t)unmasked;
		else
		{ //raise alarm we can choose to restart the computations
			out[i] = 0x00;
			status = 0;
		}
		for (i = 0; i < 16; i++){
			for (j = 0; j < n; j++){
				stateshare[i][j] = 0;
			}
		}
		// memset(stateshare[i], 0);
		//free(stateshare[i]);
	}
	return status; //everything went well
}

void inv_aes_share(const uint8_t in[16], uint8_t out[16], const uint8_t key[16])
{

	int i,j;
	uint8_t w[176];
	uint8_t wshare[176][n];

	keyexpansion((uint8_t *)key, w);

	for (i = 0; i < 176; i++)
	{
		//wshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t));
		mask(wshare[i], w[i]);
	}

	inv_aes_share_subkeys(in, out, wshare);

	for (i = 0; i < 176; i++)
	{
		for (j = 0; j < n; j++){
			wshare[i][j] = 0;
		}
		// memset(wshare[i], 0);
		// free(wshare[i]);
	}
}

/**
 * @brief AES with shares and in 10 rounds. 
 * Chipher function in AES standard
 * 
 * @param in plaitext
 * @param out result
 * @param wshare shared sub-keys
 * @param n nb of shares
 * @param k duplicating parameter
 * @return the status 1 if OK and 0 if fault detected
 */
int aes_share_subkeys(const uint8_t in[16], uint8_t out[16], uint8_t wshare[176][n])
{
	int i, j;
	int round = 0;
	uint8_t stateshare[16][n];

	for (i = 0; i < 16; i++)
	{
		// stateshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t));
		mask(stateshare[i], in[i]);
	}
	if (CIPHER_VERBOSE)
	{
		sprintf(msg, "\n**round %d\n", round);
		puts(msg);
		printMes("input", in);
	}
	addroundkey_share(stateshare, wshare, round);

	if (CIPHER_VERBOSE)
		unmaskPrintMes(stateshare, "add round key");
	for (round = 1; round < 10; round++)
	{
		if (CIPHER_VERBOSE){
			sprintf(msg, "\n**round %d\n", round);
			puts(msg);
		}
			
		subbyte_share(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "subbyte");
		shiftrows_share(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "shift rows");
		mixcolumns_share(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "mix columns");
		addroundkey_share(stateshare, wshare, round);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "add round key");
	}

	subbyte_share(stateshare);
	shiftrows_share(stateshare);
	addroundkey_share(stateshare, wshare, 10);

	int status = 1; //determine if computaion is correct (not faulted)

	for (i = 0; i < 16; i++)
	{ //check for faults before proceeding

		int unmasked = unmask(stateshare[i]);
		if (unmasked != NULL_BYTE) //TODO : leakage !!
			out[i] = (uint8_t)unmasked;
		else
		{ //raise alarm we can choose to restart the computations
			out[i] = 0x00;
			status = 0;
		}
		for (j = 0; j < n; j++){
			stateshare[i][j] = 0;
		}
		// memset(stateshare[i], 0);
		// free(stateshare[i]);
	}
	return status; //everything went well
}
int aes_share_subkeys_hardware(const uint8_t in[16], uint8_t out[16], uint8_t wshare[176][n])
{
	int i, j;
	int round = 0;
	uint8_t stateshare[16][n];

	for (i = 0; i < 16; i++)
	{
		// stateshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t));
		mask_hardware(stateshare[i], in[i]);
	}
	if (CIPHER_VERBOSE)
	{
		sprintf(msg, "\n**round %d\n", round);
		puts(msg);
		printMes("input", in);
	}
	addroundkey_share(stateshare, wshare, round);

	if (CIPHER_VERBOSE)
		unmaskPrintMes(stateshare, "add round key");
	for (round = 1; round < 10; round++)
	{
		if (CIPHER_VERBOSE){
			sprintf(msg, "\n**round %d\n", round);
			puts(msg);
		}
		subbyte_share_hardware(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "subbyte");
		shiftrows_share(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "shift rows");
		mixcolumns_share(stateshare);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "mix columns");
		addroundkey_share(stateshare, wshare, round);
		if (CIPHER_VERBOSE)
			unmaskPrintMes(stateshare, "add round key");
	}

	subbyte_share_hardware(stateshare);
	shiftrows_share(stateshare);
	addroundkey_share(stateshare, wshare, 10);

	int status = 1; //determine if computaion is correct (not faulted)

	for (i = 0; i < 16; i++)
	{ //check for faults before proceeding

		int unmasked = unmask_hardware(stateshare[i]);
		if (unmasked != NULL_BYTE) //TODO : leakage !!
			out[i] = (uint8_t)unmasked;
		else
		{ //raise alarm we can choose to restart the computations
			out[i] = 0x00;
			status = 0;
		}
		for (j = 0; j < n; j++){
			stateshare[i][j] = 0;
		}
		// memset(stateshare[i], 0);
		// free(stateshare[i]);
	}
	return status; //everything went well
}
//key expansion and then delegating to aes_share_subkeys()
//put in status 0 if faults detected and 1 ovewise
//return the time without the key expansion
uint32_t run_aes_share(const uint8_t in[16], uint8_t out[16], const uint8_t key[16], int *status, int nt)
{
	int i, j, counter;
	uint8_t w[176]; //Nb*(Nr + 1)
	uint8_t wshare[176][n];
	uint32_t start, end;
	counter = 0;
	keyexpansion((uint8_t *)key, w); //TODO key schedule not protected

	for (i = 0; i < 176; i++)
	{
		// wshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t)); //shares of the sub-keys
		mask(wshare[i], w[i]);
	}

	asm volatile ("csrr %0, mcycle" : "=r" (start));
	for (i = 0; i < nt; i++)
	{
		*status = aes_share_subkeys(in, out, wshare);
		if (*status == 0){
			// sprintf(msg, "Faults detected => need to restart over\n");
			counter++;
		}
		// else
		// 	sprintf(msg, "Everything went well\n");
	}
	// sprintf(msg, "Status: %d\tFault: %d / %d.\n",*status, counter, nt);
	// puts(msg);
	asm volatile ("csrr %0, mcycle" : "=r" (end));

	for (i = 0; i < 176; i++)
	{
		for (j = 0; j < n; j++){
			wshare[i][j] = 0;
		}
		// memset(wshare[i], 0);
		// free(wshare[i]);
	}
	//return status;
	// sprintf(msg, "run_aes_share:\t%010lu_%010lu\n",(uint32_t)(dt>>32), (uint32_t)(dt));
	// puts(msg);
	return (end - start);
}


//IPM-FD protected key expansion and then 10 rounds of AES
//return status= 0 if faults detected and 1 ovewise
int AES_IPM_FD(const uint8_t in[16], uint8_t out[16], const uint8_t key[16])
{
	int i, j, status;
	uint8_t wshare[176][n];

	// for (i = 0; i < 176; i++)
	// {
	// 	wshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t)); //shares of the sub-keys
	// 	if (!wshare[i])
	// 		error("malloc failed at wshare\n");
	// }
	AES_IPM_FD_Key_Expansion(key, wshare);

	status = aes_share_subkeys(in, out, wshare);
	for (i = 0; i < 176; i++)
	{
		for (j = 0; j < n; j++){
			wshare[i][j] = 0;
		}
		// memset(wshare[i], 0);
		// free(wshare[i]);
	}
	return status;
}

void printMes(const char *s, const uint8_t *m)
{
	sprintf(msg, "%s = ", s);
	puts(msg);
	int i;
	for (i = 0; i < 16; i++){
		sprintf(msg, "%02x", m[i]);
		puts(msg);
	}
	sprintf(msg, "\n");
	puts(msg);
}

void unmaskPrintMes(uint8_t stateshare[16][n], const char *s)
{
	int i;
	uint8_t out[16];
	for (i = 0; i < 16; i++)
	{
		out[i] = unmask(stateshare[i]);
	}
	printMes(s, out);
}

/**
 * @brief protected AES-128 key expansion
 * @param key the 128-bit key
 * @param wshare the subkeys = result of this procedure
 * @param n number of shares
 * @pama k duplicating parameter
 */
void AES_IPM_FD_Key_Expansion(const uint8_t *key, uint8_t wshare[176][n])
{
	int i, j;
	uint8_t temp[4][n];
	uint8_t rcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
	//uint8_t rcon[10];
	//setrcon(rcon);

	for (i = 0; i < 16; i++)
		mask(wshare[i], key[i]);

	uint8_t w_temp[n];
	for (i = 16; i < 176; i += 4)
	{
		for (j = 0; j < 4; j++)
			memcpy(temp[j], wshare[i - 4 + j], n);

		if ((i % 16) == 0)
		{
			sbox_share(w_temp, wshare[i - 3]);
			IPConstAdd(temp[0], w_temp, rcon[i / 16 - 1]);

			sbox_share(temp[1], wshare[i - 2]);
			sbox_share(temp[2], wshare[i - 1]);
			sbox_share(temp[3], wshare[i - 4]);
		}

		for (j = 0; j < 4; j++)
			IPM_FD_Add(wshare[i + j], wshare[i + j - 16], temp[j]);
	}
}

uint32_t run_aes_share_hardware(const uint8_t in[16], uint8_t out[16], const uint8_t key[16], int *status, int nt)
{
	int i, j, counter;
	uint8_t w[176]; //Nb*(Nr + 1)
	uint8_t wshare[176][n];
	uint32_t start, end;
	counter = 0;
	keyexpansion((uint8_t *)key, w); //TODO key schedule not protected

	for (i = 0; i < 176; i++)
	{
		// wshare[i] = (uint8_t *)malloc(n * sizeof(uint8_t)); //shares of the sub-keys
		mask_hardware(wshare[i], w[i]);
	}

	asm volatile ("csrr %0, mcycle" : "=r" (start));
	for (i = 0; i < nt; i++)
	{
		*status = aes_share_subkeys_hardware(in, out, wshare);
		if (*status == 0){
			// sprintf(msg, "Faults detected => need to restart over\n");
			counter++;
		}
		// else
		// 	sprintf(msg, "Everything went well\n");
	}
	// sprintf(msg, "Status: %d\tFault: %d / %d.\n",*status, counter, nt);
	// puts(msg);
	asm volatile ("csrr %0, mcycle" : "=r" (end));

	for (i = 0; i < 176; i++)
	{
		for (j = 0; j < n; j++){
			wshare[i][j] = 0;
		}
		// memset(wshare[i], 0);
		// free(wshare[i]);
	}
	//return status;
	return (end - start);
}
