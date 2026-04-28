#pragma once
#include <cstring>

#define uchar unsigned char
#define MAX_LEN 1008

void aes_128_encrypt(const uchar plaintext[], int input_len, const uchar key[16], uchar ciphertext[], int* final_len);
void key_expansion(const uchar key[], uchar round_keys[], int nk, int nr);
void cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr);

void aes_128_decrypt(const uchar ciphertext[], int cipher_len, const uchar key[16], uchar plaintext[], int* final_len);
void inv_cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr);

void initialize_sbox();
void print_sbox();