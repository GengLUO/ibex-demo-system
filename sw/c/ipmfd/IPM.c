// Implement Extended Inner Product Masking with faults detection : See PROOF submission (http://www.proofs-workshop.org/2019/)

// In all the files of this project
// n is the number of shares
// k is duplicating parameter for faults detection ( n > k)
// N = n - k + 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
//#include <sodium.h> //for random numbers

#include "aes.h"
#include "IPM.h"
#include "timer.h"
#include "demo_system.h"
#include "dev_access.h"

//precomputed squaring table
uint8_t sq[256] = {
    0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15,
    0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55,
    0x1b, 0x1a, 0x1f, 0x1e, 0x0b, 0x0a, 0x0f, 0x0e,
    0x5b, 0x5a, 0x5f, 0x5e, 0x4b, 0x4a, 0x4f, 0x4e,
    0x6c, 0x6d, 0x68, 0x69, 0x7c, 0x7d, 0x78, 0x79,
    0x2c, 0x2d, 0x28, 0x29, 0x3c, 0x3d, 0x38, 0x39,
    0x77, 0x76, 0x73, 0x72, 0x67, 0x66, 0x63, 0x62,
    0x37, 0x36, 0x33, 0x32, 0x27, 0x26, 0x23, 0x22,
    0xab, 0xaa, 0xaf, 0xae, 0xbb, 0xba, 0xbf, 0xbe,
    0xeb, 0xea, 0xef, 0xee, 0xfb, 0xfa, 0xff, 0xfe,
    0xb0, 0xb1, 0xb4, 0xb5, 0xa0, 0xa1, 0xa4, 0xa5,
    0xf0, 0xf1, 0xf4, 0xf5, 0xe0, 0xe1, 0xe4, 0xe5,
    0xc7, 0xc6, 0xc3, 0xc2, 0xd7, 0xd6, 0xd3, 0xd2,
    0x87, 0x86, 0x83, 0x82, 0x97, 0x96, 0x93, 0x92,
    0xdc, 0xdd, 0xd8, 0xd9, 0xcc, 0xcd, 0xc8, 0xc9,
    0x9c, 0x9d, 0x98, 0x99, 0x8c, 0x8d, 0x88, 0x89,
    0x9a, 0x9b, 0x9e, 0x9f, 0x8a, 0x8b, 0x8e, 0x8f,
    0xda, 0xdb, 0xde, 0xdf, 0xca, 0xcb, 0xce, 0xcf,
    0x81, 0x80, 0x85, 0x84, 0x91, 0x90, 0x95, 0x94,
    0xc1, 0xc0, 0xc5, 0xc4, 0xd1, 0xd0, 0xd5, 0xd4,
    0xf6, 0xf7, 0xf2, 0xf3, 0xe6, 0xe7, 0xe2, 0xe3,
    0xb6, 0xb7, 0xb2, 0xb3, 0xa6, 0xa7, 0xa2, 0xa3,
    0xed, 0xec, 0xe9, 0xe8, 0xfd, 0xfc, 0xf9, 0xf8,
    0xad, 0xac, 0xa9, 0xa8, 0xbd, 0xbc, 0xb9, 0xb8,
    0x31, 0x30, 0x35, 0x34, 0x21, 0x20, 0x25, 0x24,
    0x71, 0x70, 0x75, 0x74, 0x61, 0x60, 0x65, 0x64,
    0x2a, 0x2b, 0x2e, 0x2f, 0x3a, 0x3b, 0x3e, 0x3f,
    0x6a, 0x6b, 0x6e, 0x6f, 0x7a, 0x7b, 0x7e, 0x7f,
    0x5d, 0x5c, 0x59, 0x58, 0x4d, 0x4c, 0x49, 0x48,
    0x1d, 0x1c, 0x19, 0x18, 0x0d, 0x0c, 0x09, 0x08,
    0x46, 0x47, 0x42, 0x43, 0x56, 0x57, 0x52, 0x53,
    0x06, 0x07, 0x02, 0x03, 0x16, 0x17, 0x12, 0x13};


uint8_t hardrandom[4][4] = {
        { 43, 65, 63, 97 },
        { 123, 1, 239, 54 },
        { 78, 76, 127, 179 },
        { 222, 48, 74, 59 }
};

//IPM settings : constant public values
uint8_t L[k][n];       // The orthogonal of n^2 matrix H
uint8_t L_prime[k][N]; // orthogonal of H with the (k - 1) 0s removed (n - k +1)^2
uint8_t L_hat[k][N][N];  //L_prime*L_prime see algo IPM_Mult for details

//number of random numbers generated
// static unsigned int randcount = 0;

//generate random non zero uint8_t using sodium library
// uint8_t random_byte()
// {
//     randcount++;
//     uint8_t b = randombytes_uniform(256);
//     while (b == 0)
//     {
//         b = randombytes_uniform(256);
//     }
//     return b;
// }
uint8_t random_uint8_t() {
    int random_number;
    uint8_t random_uint8_t;
    do {
        random_number = rand();
        random_uint8_t = (unsigned char)(random_number % 256); // 生成随机数，并转换为 unsigned char 类型
    } while (random_uint8_t == 0);
    return random_uint8_t;
}
void randomuint8_ts(uint8_t *input_uint8_t, int _N) {
    for (int i = 0; i < _N; ++i) {
        input_uint8_t[i] = random_uint8_t(); // 将随机字节存储到指定的数组中
    }
}

//return the current number of used random numbers
// unsigned int get_randcount()
// {
//     return randcount;
// }

/**
 * @brief Setup the IPM-FD masking scheme
 * 
 * @param n : the number of shares
 * @patam k duplicating parameter for faults detection ( n > k)
 */
void IPM_FD_Setup()
{
    if (n <= 0 || k <= 0 || n < k)
        error("n > 0 should be greater than k > 0\n");
    int i = 0;
    // randcount = 0;
    // while (sodium_init() < 0) //init sodium library
    // {
    //     i++;
    //     if (i >= 5)
    //         error("panic! the sodium library couldn't be initialized, it is not safe to use\n");
    // }
    // L = allocate2D(k, n);
    int j;
    //use best known linear code for L (aka dual of H)

    if (n == 1 && k == 1)
    {
        L[0][0] = 1;
    }
    else if (n == 2 && k == 1)
    { //best known L for n=2 and k=1
        // 27 = X^8 = X ^ 4 + X ^ 3 + X + 1
        L[0][0] = 1;
        L[0][1] = 27;
    }
    else if (n == 3 && k == 1)
    { // 250 = X^26 = X^7 + X^6 + X^5 + X^4 + X^3 + X
        L[0][0] = 1;
        L[0][1] = 27;
        L[0][2] = 250;
    }
    else if (n == 4 && k == 1)
    { //188 = X^17 = X^7 + X^5 + X^4 + X^3 + X^2
        L[0][0] = 1;
        L[0][1] = 27;
        L[0][2] = 250;
        L[0][3] = 188;
    }
    else if (n == 2 && k == 2)
    {
        L[0][0] = 1;
        L[0][1] = 0;

        L[1][0] = 0;
        L[1][1] = 1;
    }
    else if (n == 3 && k == 2)
    { //best known L for n=3 and k=2
        L[0][0] = 1;
        L[0][1] = 0;
        L[0][2] = 27;

        L[1][0] = 0;
        L[1][1] = 1;
        L[1][2] = 188;
    }
    else if (n == 4 && k == 2)
    { //151 = x^20 = X^7 + X^4 + X^2 + X + 1
        //239 = x^27 = X^7 + X^6 + X^5 + X^3 + X^2 + X + 1
        // 128 = X^7
        L[0][0] = 1;
        L[0][1] = 0;
        L[0][2] = 27;
        L[0][3] = 151;

        L[1][0] = 0;
        L[1][1] = 1;
        L[1][2] = 239;
        L[1][3] = 128;
    }
    else if (n==5 && k == 2) //TODO: DELETE IT
    {
        L[0][0] = 1;
        L[0][1] = 0;
        L[0][2] = 43;
        L[0][3] = 122;
        L[0][4] = 199;

        L[1][0] = 0;
        L[1][1] = 1;
        L[1][2] = 27;
        L[1][3] = 250;
        L[1][4] = 188;
    }
    else
    {
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < k; j++)
                L[i][j] = (i == j) ? 1 : 0;

            //choose the best values here
            for (j = k; j < n; j++)
                L[i][j] = random_uint8_t(); //make sure they all non-zero
        }
    }
    // int N = n - k + 1;
    // L_prime = allocate2D(k, N);

    for (j = 0; j < k; j++)
    {
        L_prime[j][0] = 1;
        for (i = 1; i < N; i++)
        {
            L_prime[j][i] = L[j][i + k - 1];
        }
    }

    // L_hat = allocate3D(k, N, N);
    int p;
    for (p = 0; p < k; p++)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                L_hat[p][i][j] = GF256_Mult(L_prime[p][i], L_prime[p][j]);
            }
        }
    }
    ipm_mult_count = 0;
    ipm_square_count = 0;
    ipm_homo_count = 0;
    ipm_mask_count = 0;
    ipm_unmask_count = 0;
    // print(L[1], "L = \t");
    // puts("L_hat =\n");
    // for(i = 0; i < N; i++)
    //     print(L_hat[1][i], "");
    // return L;
}

//generate the squaring table for lookup
void gen_square_table()
{
    int i;
    uint8_t x = 0;
    puts("uint8_t sq[256]={");
    for (i = 0; i < 256; i++)
    {
        if ((i % 8) == 0){
            puts("\n");
        }
        sprintf(msg, "0x%02x", GF256_Mult(x, x));
        puts(msg);
        x++;
        if (i < 255){
            puts(",");
        }
    }
    puts("};\n");
}

uint8_t GF256_Square(uint8_t x)
{
    //return GF256_Mult(x,x);
    return sq[x];
}

//Iterative constant time multiplication over GF(2^8) with unconditional reduction
//see paper "On the Performance and Security of Multiplication in GF(2^N)"
uint8_t GF256_Mult(uint8_t a, uint8_t b)
{
    int x = a, y = b, m, res = 0;
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        m = -(y & 1); //m is either 0xffff or 0x0000
        res ^= (x & m);
        y >>= 1;
        x <<= 1;
        //x must be reduced mod 0x011b = x^8+x^4+x^3+x+1
        m = -((x >> 8) & 1); //MSB
        x ^= (m & 0x1b);
    }
    return (uint8_t)res;
}

void IPM_Square(uint8_t *x2, const uint8_t *x, int position)
{
    int i;
    for (i = 1; i < N; i++)
        x2[i] = GF256_Mult(GF256_Square(x[i]), L_prime[position][i]);
    x2[0] = GF256_Square(x[0]); //L_prime[position][0] = 1

    // sprintf(msg, "Square position = %d\n", position);
    // puts(msg);
    // print(x, "x:\t");
    // print(x2, "x^2:\t");
    // puts("\n");
}

void IPM_Square_hardware(uint8_t *x2, const uint8_t *x, int position){
    ipm_square_count++;
    uint32_t a_32, b_32, res_32;
    b_32 = 0;
    a_32 = expandToUint32((uint8_t*)x);

    asm (".insn r CUSTOM_1, 2, 0, %0, %1, %2" : // func3 == 3'b010
           "=r"(res_32) :
           "r"(a_32), "r"(b_32));

    restoreToArray(x2, res_32);
}
//squaring of IPM_FD share, more efficient than Mult(x,x)
void IPM_FD_Square(uint8_t *Z2, const uint8_t *Z)
{
    int i, j;//, N = n - k + 1;
    uint8_t P[k][N];
    uint8_t Q[k - 1];

    uint8_t Z__[k][N];

    for (i = 0; i < k; i++)
    {
        Z__[i][0] = Z[i];
        memcpy(Z__[i] + 1, Z + k, n - k); //copy first i-1 coord of Z
    }
    for (j = 0; j < k; j++)
        IPM_Square(P[j], Z__[j], j);

    //No need to refresh because IPM_Mult already does the job
    //for (j = 1; j < k; j++) //refresh only k-1 of them
    //    IPRefresh(P[j], N, j);

    for (j = 0; j < k - 1; j++)
        Q[j] = IPM_Homogenize(P[0], P[j + 1], j + 1);

    //return
    Z2[0] = P[0][0];
    memcpy(Z2 + k, P[0] + 1, n - k);
    for (j = 0; j < k - 1; j++)
    {
        Z2[j + 1] = Q[j];
    }
}
void IPM_FD_Square_hardware(uint8_t *Z2, const uint8_t *Z)
{
    int i, j;//, N = n - k + 1;
    uint8_t P[k][N];
    uint8_t Q[k - 1];

    uint8_t Z__[k][N];

    for (i = 0; i < k; i++)
    {
        Z__[i][0] = Z[i];
        memcpy(Z__[i] + 1, Z + k, n - k); //copy first i-1 coord of Z
    }
    for (j = 0; j < k; j++)
        IPM_Square_hardware(P[j], Z__[j], j);

    //No need to refresh because IPM_Mult already does the job
    //for (j = 1; j < k; j++) //refresh only k-1 of them
    //    IPRefresh(P[j], N, j);

    for (j = 0; j < k - 1; j++)
        Q[j] = IPM_Homogenize_hardware(P[0], P[j + 1], j + 1);

    //return
    Z2[0] = P[0][0];
    memcpy(Z2 + k, P[0] + 1, n - k);
    for (j = 0; j < k - 1; j++)
    {
        Z2[j + 1] = Q[j];
    }
}

//algo 2 : secure addition in IPM-FD
void IPM_FD_Add(uint8_t *res, const uint8_t *op1, const uint8_t *op2)
{
    int i;
    for (i = 0; i < n; i++)
    {
        res[i] = op1[i] ^ op2[i];
    }
}

void IPM_FD_Mult(uint8_t *R, const uint8_t *Z, const uint8_t *Z_prime)
{
    int i, j;//, N = n - k + 1;
    uint8_t P[k][N];

    uint8_t Z__[k][N], Z_prime__[k][N]; //Z__[i] => i th in [0...k-1] coord of Z is kept, the other in [0...k-1] are dropped;

    for (i = 0; i < k; i++)
    {
        Z__[i][0] = Z[i];
        memcpy(Z__[i] + 1, Z + k, n - k); //copy first i-1 coord of Z

        Z_prime__[i][0] = Z_prime[i];
        memcpy(Z_prime__[i] + 1, Z_prime + k, n - k);
    }
    
    for (j = 0; j < k; j++)
        IPM_Mult(P[j], Z__[j], Z_prime__[j], j);

    for (j = 1; j < k; j++)
        P[j][0] = IPM_Homogenize(P[0], P[j], j);

    //return
    for (j = 0; j < k; j++)
        R[j] = P[j][0];
    memcpy(R + k, P[0] + 1, n - k);
}

void IPM_FD_Mult_hardware(uint8_t *R, const uint8_t *Z, const uint8_t *Z_prime)
{
    int i, j;//, N = n - k + 1;
    uint8_t P[k][N];

    uint8_t Z__[k][N], Z_prime__[k][N]; //Z__[i] => i th in [0...k-1] coord of Z is kept, the other in [0...k-1] are dropped;

    for (i = 0; i < k; i++)
    {
        Z__[i][0] = Z[i];
        memcpy(Z__[i] + 1, Z + k, n - k); //copy first i-1 coord of Z

        Z_prime__[i][0] = Z_prime[i];
        memcpy(Z_prime__[i] + 1, Z_prime + k, n - k);
    }

    for (j = 0; j < k; j++){
        IPM_Mult_hardware(P[j], Z__[j], Z_prime__[j], j);
        // print(P[j], "P:");
    }
        
    for (j = 1; j < k; j++)
        P[j][0] = IPM_Homogenize_hardware(P[0], P[j], j);

    //return
    for (j = 0; j < k; j++)
        R[j] = P[j][0];
    memcpy(R + k, P[0] + 1, n - k);
}

//Add constant value to masked vector
void IPConstAdd(uint8_t *res, const uint8_t *x, uint8_t c)
{
    memcpy(res + k, x + k, n - k);
    int i;
    for (i = 0; i < k; i++)
        res[i] = x[i] ^ c;
}

//Multiply constant value to masked vector
void IPConstMult(uint8_t *res, const uint8_t *x, uint8_t c)
{
    int i;
    for (i = 0; i < n; i++)
    {
        res[i] = GF256_Mult(x[i], c);
    }
}

void IPM_Mult(uint8_t *P, const uint8_t *Z, const uint8_t *Z_prime, int position)
{
    //we have here n-k+1 compare to the original n in IPMsetup()
    int i, j;

    //step 1
    uint8_t A_hat[N][N];
    // for (i = 0; i < N - 1; i++)
    //     randomuint8_ts(A_hat[i], N);
    // randomuint8_ts(A_hat[N - 1], N - 1);
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            A_hat[i][j] = hardrandom[i][j];

    // randcount += N * N - 1;
    uint8_t delta = 0;
    for (j = 0; j < N - 1; j++)
    {
        uint8_t sum = 0;
        for (i = 0; i < N; i++)
        {
            sum ^= GF256_Mult(A_hat[i][j], L_hat[position][i][j]);
        }
        delta ^= sum;
    }

    uint8_t sum = 0;
    for (i = 0; i < N - 1; i++)
    {
        sum ^= GF256_Mult(A_hat[i][N - 1], L_hat[position][i][N - 1]);
    }
    A_hat[N - 1][N - 1] = GF256_Mult(delta ^ sum, GF256_Inverse(L_hat[position][N - 1][N - 1]));

    //step 2
    uint8_t R_hat[N][N];
    //save values for i,j in {k, N}

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
            R_hat[i][j] = GF256_Mult(Z[i], Z_prime[j]);
    }

    //step 3
    uint8_t B_hat[N][N];
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            B_hat[i][j] = R_hat[i][j] ^ A_hat[i][j];
        }
    }
    //step 4
    uint8_t b = 0;
    for (i = 1; i < N; i++)
    {
        uint8_t tmp = 0;
        for (j = 0; j < N; j++)
        {
            tmp ^= GF256_Mult(L_hat[position][i][j], B_hat[i][j]);
        }
        b ^= tmp;
    }
    //return
    memcpy(P, B_hat[0], N);
    P[0] ^= b;
}
void IPM_Mult_hardware(uint8_t *P, const uint8_t *Z, const uint8_t *Z_prime, int position)
{
    ipm_mult_count++;
    uint32_t a_32, b_32, res_32;
    a_32 = expandToUint32((uint8_t*)Z);
    b_32 = expandToUint32((uint8_t*)Z_prime);
    asm (".insn r CUSTOM_1, 0, 0, %0, %1, %2" :
           "=r"(res_32) :
           "r"(a_32), "r"(b_32));

    restoreToArray(P, res_32);
}

uint8_t IPM_Homogenize(const uint8_t *Z, const uint8_t *Z_prime, int position)
{
    int i;
    uint8_t res = Z_prime[0];
    // uint8_t epsilon;
    for (i = 1; i < N; i++)
        res ^= GF256_Mult(L_prime[position][i], Z[i] ^ Z_prime[i]);
    return res;
}
uint8_t IPM_Homogenize_hardware(const uint8_t *Z, const uint8_t *Z_prime, int position)
{
    ipm_homo_count++;
    uint32_t a_32, b_32, res_32;
    a_32 = expandToUint32((uint8_t*)Z);
    b_32 = expandToUint32((uint8_t*)Z_prime);
    
    asm (".insn r CUSTOM_1, 1, 0, %0, %1, %2" : // func3 == 3'b001: IPM_OP_HOMOG
           "=r"(res_32) :
           "r"(a_32), "r"(b_32));

    // sprintf(msg, "Z: %lu\t Z_prime %lu\n", a_32, b_32);
    // puts(msg);
    return (uint8_t)(res_32>>24);
}
/**
 * @brief inner product between the mask M and param L
 *  
 * @param Z  = sum (L<i+k> * Mi)
 * @param M (the mask) has size = n-k
 */
void innerProduct(uint8_t *Z, uint8_t *M)
{
    int i, j;
    for (j = 0; j < k; j++)
    {
        Z[j] = 0;
        for (i = 0; i < n - k; i++)
            Z[j] ^= GF256_Mult(L[j][i + k], M[i]); //Field multiplication
    }
    for (i = k; i < n; i++)
        Z[i] = M[i - k];
}

void mask(uint8_t *Z, uint8_t X)
{
    int i;
    uint8_t M[n - k]; //n - k random masks
    randomuint8_ts(M, n - k);
    // randcount += n - k;
    innerProduct(Z, M);
    for (i = 0; i < k; i++)
        Z[i] ^= X; //duplicate the data
}
void mask_hardware(uint8_t *Z, uint8_t X)
{
    ipm_mask_count++;
    uint32_t  X_32, Y_32;
    uint8_t tmp[k][N];
    int i;
    X_32 = ((uint32_t)X << 24);
    
    for (i = 0; i < k; i++){
        asm (".insn r CUSTOM_1, 7, 0, %0, %1, %2" : // func3 == 3'b111 for masking
        "=r"(Y_32) :
        "r"(X_32), "r"(Y_32));
        restoreToArray(tmp[i], Y_32);
        // print(tmp[i], "mask_tmp:");
    }
    for (i = 0; i < k; i++){
        Z[i] = tmp[i][0];
    }
    memcpy(Z + k, tmp[0] + 1, n - k);
    
    
}

//return 1 if detected faults and 0 overwise
//TODO return where the fault is
int detect_faults(const uint8_t *X)
{
    int i, flag = 0;
    for (i = 1; i < k; i++)
        if (X[0] != X[i])
        {
            // sprintf(msg, "***Fault deteted : %d, %d\n", X[0], X[1]);
            // puts(msg);
            flag = 1;
            break;
        }
    return flag;
}

//conditional swap
//return a if bool==1, b overwise
int cswap(int a, int b, int boo)
{
    return boo * a + (1 - boo) * b;
}

int unmask(const uint8_t Z[])
{
    int i, j;
    uint8_t res[k];
    for (i = 0; i < k; i++){
        res[i] = 0;
    }
    // memset(res, 0, k);
    for (j = 0; j < k; j++)
    {
        for (i = 0; i < n; i++)
            res[j] ^= GF256_Mult(L[j][i], Z[i]);
    }
    // print(res, "unmask_array_sw:");
    //check for faults
    i = detect_faults(res);
    //return NULL_BYTE if i=1 (fault detected) and correct uint8_t otherwise
    return cswap(NULL_BYTE, res[0], i);
}

int unmask_hardware(const uint8_t Z[])
{
    int i;
    ipm_unmask_count++;
    uint32_t X_32, Z_32;
    uint8_t res[k];
    uint8_t tmp[k][N];

    for (i = 0; i < k; i++){
        tmp[i][0] = Z[i];
        memcpy(tmp[i] + 1, Z + k, N - 1);
        Z_32 = expandToUint32(tmp[i]);
        asm (".insn r CUSTOM_1, 4, 0, %0, %1, %2" : // func3 == 3'b100 for unmasking
            "=r"(X_32) :
            "r"(Z_32), "r"(Z_32));
        // sprintf(msg, "X_32: %lx ", X_32);
        // puts(msg);
        res[i] = (uint8_t)(X_32 >> 24);
    }
    
    i = detect_faults(res);
    return cswap(NULL_BYTE, res[0], i);
}

void IPRefresh(uint8_t *Z, int position)
{
    uint8_t R[N];
    randomuint8_ts(R + 1, N - 1); //n-k random bytes
    // randcount += N - 1;
    int i;
    R[0] = 0;
    for (i = 1; i < N; i++)
        R[0] ^= GF256_Mult(R[i], L_prime[position][i]);

    IPM_FD_Add(Z, Z, R);
}

void IPM_FD_Refresh(uint8_t *Z)
{
    int i, j;
    uint8_t epsilon[n - k], Z_prime[n];
    randomuint8_ts(epsilon, n - k);
    // randcount += n - k;

    for (i = k; i < n; i++)
        Z_prime[i] = Z[i] ^ epsilon[i - k];

    for (i = 0; i < k; i++)
    {
        Z_prime[i] = Z[i];
        for (j = 0; j < n - k; j++)
            Z_prime[i] ^= GF256_Mult(L[i][j + k], epsilon[j]); //Field multiplication
    }
    memcpy(Z, Z_prime, n);
}

void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

//allocate a 3D array
// uint8_t ***allocate3D(int k, int m)
// {
//     uint8_t ***arr3D;
//     int i, j;

//     arr3D = (uint8_t ***)malloc(k * sizeof(uint8_t **));
//     if (!arr3D)
//         error("malloc failed at allocate3D\n");
//     for (i = 0; i < k; i++)
//     {
//         arr3D[i] = (uint8_t **)malloc(m * sizeof(uint8_t *));
//         if (!arr3D[i])
//             error("malloc failed at allocate3D\n");
//         for (j = 0; j < m; j++)
//         {
//             arr3D[i][j] = (uint8_t *)malloc(n * sizeof(uint8_t));
//             if (!arr3D[i][j])
//                 error("malloc failed at allocate3D\n");
//         }
//     }

//     return arr3D;
// }

//deallocate a 3D array
// void deallocate3D(uint8_t ***arr3D, int m, int p)
// {
//     int i, j;

//     for (i = 0; i < k; i++)
//     {
//         for (j = 0; j < m; j++)
//         {
//             memset(arr3D[i][j], 0, p);
//             free(arr3D[i][j]);
//         }
//         memset(arr3D[i], 0, m);
//         free(arr3D[i]);
//     }
//     memset(arr3D, 0, k);
//     free(arr3D);
// }

//allocate a 2D array
// uint8_t **allocate2D(int rows, int cols)
// {
//     uint8_t **arr2D;
//     int i;

//     arr2D = (uint8_t **)malloc(rows * sizeof(uint8_t *));
//     for (i = 0; i < rows; i++)
//     {
//         arr2D[i] = (uint8_t *)malloc(cols * sizeof(uint8_t));
//     }
//     return arr2D;
// }

//deallocate a 2D array
// void deallocate2D(uint8_t **arr2D, int rows, int cols)
// {
//     int i;
//     for (i = 0; i < rows; i++)
//     {
//         memset(arr2D[i], 0, cols);
//         free(arr2D[i]);
//     }
//     memset(arr2D, 0, rows);
//     free(arr2D);
// }

// void freeMemory(int n)
// {
//     int N = n - k + 1;
//     deallocate2D(L, k, n);
//     deallocate2D(L_prime, k, n);
//     deallocate3D(L_hat, k, N, N);
// }
void print(const uint8_t *a, const char *msg0)
{
    int j;
    sprintf(msg, "%s[", msg0);
    puts(msg);
    for (j = 0; j < n - 1; j++)
    {
        sprintf(msg, "%02x,", a[j]);
        puts(msg);
    }
    sprintf(msg, "%02x]\n", a[n - 1]);
    puts(msg);
}

void print_setup_info(){
    puts("+-+-+-+-+-Setup Information-+-+-+-+\n");
    puts("||IPM_MUL_Hardware:\t");
    if(IPM_MUL_HW) puts("enabled  ||\n" );
              else puts("disabled ||\n");
        

    puts("||IPM_SQUARE_Hardware:\t");
    if(IPM_SQUARE_HW) puts("enabled  ||\n" );
                 else puts("disabled ||\n");

    puts("||IPM_HOMO_Hardware:\t");
    if(IPM_HOMO_HW) puts("enabled  ||\n" );
               else puts("disabled ||\n");

    puts("||IPM_MASK_Hardware:\t");
    if(IPM_MASK_HW) puts("enabled  ||\n" );
               else puts("disabled ||\n");

    puts("||IPM_UNMASK_Hardware:\t");
    if(IPM_UNMASK_HW) puts("enabled  ||\n" );
               else puts("disabled ||\n");
    puts("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
}