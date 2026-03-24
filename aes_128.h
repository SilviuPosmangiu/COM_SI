#pragma once
#include <cstring>

#define uchar unsigned char

void aes_128_encrypt(const uchar plaintext[16], const uchar key[16], uchar ciphertext[16]);
void key_expansion(const uchar key[], uchar round_keys[], int nk, int nr);
void cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr);

void aes_128_decrypt(const uchar ciphertext[16], const uchar key[16], uchar plaintext[16]);
void inv_cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr);

void initialize_sbox();
void print_sbox();