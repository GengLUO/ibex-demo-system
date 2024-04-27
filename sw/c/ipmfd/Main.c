#include <stdio.h>
// #include <time.h>
#include <string.h>

#include "AES_IPM.h"
#include "IPM.h"
#include "aes.h"
#include "common_IPM.h"

#define TEST 0
void test()
{
	
	// int n = 10,
	// 	k = 5; //Nb of shares; duplicating parameter
	IPM_FD_Setup();
	sprintf(msg, "--------IPM-FD n = %d, k = %d--------\n ", n, k);
	puts(msg);
	for (int i = 0; i < k; i++)
		print(L[i], "");
	puts("----------------------------------\n");

	//test mask | unmask
	uint8_t X = 33;//random_uint8_t();

	sprintf(msg, "X = %d\n", X);
	puts(msg);

	uint8_t Z[n];
	mask_hardware(Z, X);
	print(Z, "masked data ");

	X = unmask_hardware(Z);
	sprintf(msg, "Unmasked = %d\n\n", X);
	puts(msg);

	uint8_t M = 0x37;
	uint8_t U[n];
	uint8_t R[n];
	mask_hardware(U, M);
	// IPM_FD_Mult_hardware(R, Z, U);
	IPM_FD_Mult_hardware(R, Z, U);
	print(R, "R:");
	int p = unmask_hardware(R);
	sprintf(msg, "Unmasked = %x\n\n", p);
	puts(msg);

	puts("---------------- Test IPM_FD_Mult --------------\n");
	uint8_t Y = X; //random_uint8_t();
	sprintf(msg, "Y = %d\n", Y);
	puts(msg);
	uint8_t Q[n];
	mask_hardware(Q, Y);
	print(Q, "(Sharing) masked data Q ");
	int u = unmask_hardware(Q);
	sprintf(msg, "Unmasked = %d\n\n", u);
	puts(msg);

	/////
	sprintf(msg, "X*Y = %d\n", GF256_Mult(X, Y));
	puts(msg);
	uint8_t T[n];

	IPM_FD_Mult_hardware(T, Z, Q);

	print(T, "Z*Q ");
	u = unmask_hardware(T);
	sprintf(msg, "Unmasked = %d\n\n", u);
	puts(msg);
	puts("------------------ End of Test -----------------\n");
}
int main()
{
	if (TEST)
		test();
	else
	{
		
		puts("\n-------------AES CIPHER------------\n");
		// uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
		// uint8_t in[16] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
		// uint8_t outex[16] = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};

		uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
		uint8_t in[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
		uint8_t outex[16] = {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
	
		printMes("in\t", in);
		printMes("key\t", key);
		uint8_t out[16];
		int i;

		for (i = 0; i < 16; i++){
			out[i] = 0;
		}
		// memset(out, 0, 16);

		// int n = 3, k = 2, 
		int nbyte = 16; //Nb of shares; duplicating parameter

		// sprintf(msg, "\n---------IPM-FD with protected Key Expansion----------\n");
		// memset(out, 0);
		// IPM_FD_Setup();
		// AES_IPM_FD(in, out, key);
		// check_ciphertext(out, outex, nbyte);
		// printMes("\nout", out);
		// freeMemory();
		// //----------------------------------

		//----------------------------------
		int status = -1, nTimes = 1;//, base = 0;
		puts("Without countermeasure:\n");
		time_costs.dt_no_counter = run_aes(in, out, key, nTimes);
		//base = dt;
		check_ciphertext(out, outex, nbyte);

		// report_time(dt, nTimes, base, 0); //randcount=0

		puts("-----------Pure Software-----------\n");
		for (i = 0; i < nbyte; i++){
			out[i] = 0;
		}
		// memset(out, 0, nbyte);
		// print_setup_info();
		sprintf(msg, "\nWith extended IPM-FD n = %d, k = %d :\n", n, k);
		puts(msg);

		IPM_FD_Setup();
		time_costs.dt_with_counter = run_aes_share(in, out, key, &status, nTimes);
		// sprintf(msg, "run_aes_share:\t%010lu_%010lu\n",(uint32_t)(time_costs.dt_with_counter>>32), (uint32_t)(time_costs.dt_with_counter));
		// puts(msg);
		if (status == 0)
		{
			puts("Faults detected => need to restart over\n");
		}
		//else
		//	sprintf(msg, "Everything went well\n");

		// report_time(dt, nTimes, base, get_randcount());
		if (check_ciphertext(out, outex, nbyte)){
			printMes("out\t", out);
			printMes("outex\t", outex);
		}
		// freeMemory(n, k);
		puts("-----------------------------------\n");
		puts("--------------Hardware-------------\n");
		for (i = 0; i < nbyte; i++){
			out[i] = 0;
		}
		// memset(out, 0, nbyte);
		// print_setup_info();
		sprintf(msg, "\nWith extended IPM-FD n = %d, k = %d :\n", n, k);
		puts(msg);

		IPM_FD_Setup();
		time_costs.dt_hardware = run_aes_share_hardware(in, out, key, &status, nTimes);
		if (status == 0)
		{
			puts("Faults detected => need to restart over\n");
		}
		//else
		//	sprintf(msg, "Everything went well\n");

		// report_time(dt, nTimes, base, get_randcount());
		if (check_ciphertext(out, outex, nbyte)){
			printMes("out\t", out);
			printMes("outex\t", outex);
		}
		// freeMemory(n, k);

		puts("-----------------------------------\n");
		report_cycle();
		puts("-----------------------------------\n\n");
	}
	while(1){}
}
