#include "functions.h"
#include <stdio.h>

#define ROTL8(x,shift) ((uchar) ((x) << (shift)) | ((x) >> (8 - (shift))))

uchar sbox[256];
bool sbox_initialized = false;

void initialize_sbox() {
	uchar p = 1, q = 1;

	do {
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

		q ^= q << 1;
		q ^= q << 2;
		q ^= q << 4;
		q ^= q & 0x80 ? 0x09 : 0;

		uchar xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

		sbox[p] = xformed ^ 0x63;
	} while (p != 1);

	sbox[0] = 0x63;

	sbox_initialized = true;
}

void print_sbox() {
	printf("S-box:\n");
	printf("sbox_initialized = %d\n", (int)sbox_initialized);
	for (int i = 0; i < 256; i++) {
		printf("%02x ", sbox[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

void copy_word(uchar dest[4], uchar src[4]) {
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
}

void rot_word(uchar a[4]) {
	uchar aux = a[0];
	a[0] = a[1];
	a[1] = a[2];
	a[2] = a[3];
	a[3] = aux;
}

void sub_word(uchar a[4]) {
	a[0] = sbox[a[0]];
	a[1] = sbox[a[1]];
	a[2] = sbox[a[2]];
	a[3] = sbox[a[3]];
}

void key_expansion(const uchar key[], uchar round_keys[], int nk, int nr) {
	int i = 0;
	while (i <= nk - 1) {
		round_keys[4 * i + 0] = key[4 * i + 0];
		round_keys[4 * i + 1] = key[4 * i + 1];
		round_keys[4 * i + 2] = key[4 * i + 2];
		round_keys[4 * i + 3] = key[4 * i + 3];
		i++;
	}
	while (i <= 4 * nr + 3) {
		uchar temp[4];
		copy_word(temp, round_keys + i - 1);
		if (i % nk == 0) {
			rot_word(temp);
			sub_word(temp);
			temp[0] ^= 
		}
	}
}

void chipher(uchar in[16], uchar out[16], uchar round_keys[], int nr) {

}

void aes_128_encrypt(const uchar plaintext[], const uchar key[], uchar ciphertext[]) {
	
}