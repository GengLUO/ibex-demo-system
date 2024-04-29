#ifndef __IMP_h__
#define __IMP_h__

typedef unsigned char uint8_t;
#define NULL_BYTE -1
#define n 5
#define k 2
#define N n-k+1

#define IPM_HOMO_HW   0
#define IPM_MUL_HW    0
#define IPM_SQUARE_HW 0
#define IPM_MASK_HW   0
#define IPM_UNMASK_HW   0

extern uint8_t L[k][n];
extern uint8_t L_prime[k][N];

/**
 * @brief Use the sodium lib RNG to get a random uint8_t
 * 
 * @return uint8_t 
 */
uint8_t random_uint8_t();
// unsigned int get_randcount();

/**
 * @brief Setup the masking scheme : choose the best code
 * for n<=4 and k<=2 we chosse the best known code H, overwise the randomly choose the code
 * 
 * @param n number of shares
 * @param k duplicating parameter
 * @return uint8_t** the dual code of H
 */
void IPM_FD_Setup();

/**
 * @brief mask a sensitive variable into a sharing w.r.t extended IPM scheme
 * 
 * @param Z result = the sharing
 * @param X sensitive value to be masked
 * @param n nb of shares
 * @param k duplicating parameter
 */
void mask(uint8_t *Z, uint8_t X);
void mask_hardware(uint8_t *Z, uint8_t X);

void IPRefresh(uint8_t *Z, int position);
/**
 * @brief refresh the mask
 * 
 * @param Z add to Z a random mask R such that <L,R>=<L,Z>
 * @param n 
 * @param k 
 */
void IPM_FD_Refresh(uint8_t *Z);

/**
 * @brief unmask a sharing
 * We can also check for data integrity (faults)
 * @param Z the sharing
 * @param n nb of shares
 * @param k we have k ways to demask
 * @return the sensitive data or NULL_BYTE if fault detected
 */
int unmask(const uint8_t Z[]);
int unmask_hardware(const uint8_t Z[]);

uint8_t GF256_Square(uint8_t x);

uint8_t GF256_Mult(uint8_t a, uint8_t b);
/**
 * @brief Secure multiplication with fault detection
 * 
 * @param R = Z x Z' masked under the same L
 * @param Z 1st operand
 * @param Z_prime 2nd operand
 */
void IPM_FD_Mult(uint8_t *R, const uint8_t *Z, const uint8_t *Z_prime);
void IPM_FD_Mult_hardware(uint8_t *R, const uint8_t *Z, const uint8_t *Z_prime);

void IPM_FD_Add(uint8_t *res, const uint8_t *op1, const uint8_t *op2);

void IPM_FD_Square(uint8_t *x2, const uint8_t *x);
void IPM_FD_Square_hardware(uint8_t *x2, const uint8_t *x);

/**
 * @brief compute squaring more efficiently to use in S-box
 * Use original IPSquare 
 * 
 * @param x2 
 * @param x 
 * @param N 
 * @param position indicate witch L to use
 */
void IPM_Square(uint8_t *x2, const uint8_t *x, int position);
void IPM_Square_hardware(uint8_t *x2, const uint8_t *x, int position);

void innerProduct(uint8_t *Z, uint8_t *M);
/**
 * @brief Homogenization of two sharings
 * 
 * @param position indicate whitch L to use w.r.t the the matrix L in extended IPM
 * @return an homegenized value of T and T'
 * @param N = n-k+1
 * @note : We improved this algo by only computing the needed value instead of the whole table R
 */
uint8_t IPM_Homogenize(const uint8_t *Z, const uint8_t *Z_prime, int position);
uint8_t IPM_Homogenize_hardware(const uint8_t *Z, const uint8_t *Z_prime, int position);

/**
 * @brief standard IPM multiplication
 * multiply masked values under same vector L
 * 
 * @param P result of mutl
 * @param Z 1st operand
 * @param Z_prime 2nd operand
 * @param N size of operands, also equals to n-k+1, (n is the original number of shares)
 * @param k current duplicating parameter
 * @param position indicate with L to use w.r.t the the matrix L in extended IPM
 */
void IPM_Mult(uint8_t *P, const uint8_t *Z, const uint8_t *Z_prime, int position);
void IPM_Mult_hardware(uint8_t *P, const uint8_t *Z, const uint8_t *Z_prime, int position);
void IPConstMult(uint8_t *res, const uint8_t *x, uint8_t c);
void IPConstAdd(uint8_t *res, const uint8_t *x, uint8_t c);

void error(const char *msg);

// uint8_t ***allocate3D(int k, int m);
// uint8_t **allocate2D(int rows, int cols);
// void deallocate2D(uint8_t **arr2D, int rows, int cols);
// void deallocate3D(uint8_t ***arr3D, int m, int p);

void print(const uint8_t *a, const char *msg);

// void freeMemory(int n);
#endif
void print_setup_info();