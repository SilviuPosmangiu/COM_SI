#pragma once
#include <cstring>

#define uchar unsigned char

void aes_128_encrypt(const uchar plaintext[], const uchar key[], uchar ciphertext[]);
void key_expansion(const uchar key[], uchar round_keys[], int nk, int nr);
void chipher(uchar in[16], uchar out[16], uchar round_keys[], int nr);

void initialize_sbox();
void print_sbox();