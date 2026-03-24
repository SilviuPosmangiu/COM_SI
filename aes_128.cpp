#include "aes_128.h"
#include <stdio.h>

#define ROTL8(x,shift) ((uchar) ((x) << (shift)) | ((x) >> (8 - (shift))))

uchar sbox[256];
uchar inv_sbox[256];
uchar rcon[] = { 0x00, 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36 };
bool sbox_initialized = false;

void initialize_sbox() {
	uchar p = 1, q = 1;

	do {
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

		q ^= q << 1;
		q ^= q << 2;
		q ^= q << 4;
		q ^= (q & 0x80) ? 0x09 : 0;

		uchar xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

		sbox[p] = xformed ^ 0x63;
	} while (p != 1);

	sbox[0] = 0x63;

	for(int i = 0; i < 256; i++) {
		inv_sbox[sbox[i]] = (uchar)i;
	}

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
	printf("Inv S-box:\n");
	printf("sbox_initialized = %d\n", (int)sbox_initialized);
	for (int i = 0; i < 256; i++) {
		printf("%02x ", inv_sbox[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

void print_state(uchar state[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%02x ", state[i][j]);
		}
		printf("\n");
	}
}

void print_roundkey(uchar* round_keys) {
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			printf("%02x ", round_keys[4 * c + r]);
		}
		printf("\n");
	}
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
		copy_word(temp, round_keys + (i - 1) * 4);
		if (i % nk == 0) {
			rot_word(temp);
			sub_word(temp);
			temp[0] ^= rcon[i / nk];
		}
		else if (nk > 6 && i % nk == 4) {
			sub_word(temp);
		}
		round_keys[4 * i + 0] = round_keys[4 * (i - nk) + 0] ^ temp[0];
		round_keys[4 * i + 1] = round_keys[4 * (i - nk) + 1] ^ temp[1];
		round_keys[4 * i + 2] = round_keys[4 * (i - nk) + 2] ^ temp[2];
		round_keys[4 * i + 3] = round_keys[4 * (i - nk) + 3] ^ temp[3];
		i++;
	}
}

void shift_rows(uchar state[4][4]) {
	uchar temp;
	// Row 1
	temp = state[1][0];
	state[1][0] = state[1][1];
	state[1][1] = state[1][2];
	state[1][2] = state[1][3];
	state[1][3] = temp;
	// Row 2
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;
	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;
	// Row 3
	temp = state[3][0];
	state[3][0] = state[3][3];
	state[3][3] = state[3][2];
	state[3][2] = state[3][1];
	state[3][1] = temp;
}

uchar gf_mult(uchar a, uchar b) {
	uchar p = 0;
	for (int i = 0; i < 8; i++) {
		if (b & 1) {
			p ^= a;
		}
		bool high_bit = a & 0x80;
		a <<= 1;
		if (high_bit) {
			a ^= 0x1B;
		}
		b >>= 1;
	}
	return p;
}

void mix_columns(uchar state[4][4]) {
	for (int c = 0; c < 4; c++) {
		uchar a[4];
		for (int i = 0; i < 4; i++) {
			a[i] = state[i][c];
		}
		state[0][c] = (uchar)(gf_mult(0x02, a[0]) ^ gf_mult(0x03, a[1]) ^ a[2] ^ a[3]);
		state[1][c] = (uchar)(a[0] ^ gf_mult(0x02, a[1]) ^ gf_mult(0x03, a[2]) ^ a[3]);
		state[2][c] = (uchar)(a[0] ^ a[1] ^ gf_mult(0x02, a[2]) ^ gf_mult(0x03, a[3]));
		state[3][c] = (uchar)(gf_mult(0x03, a[0]) ^ a[1] ^ a[2] ^ gf_mult(0x02, a[3]));
	}
}

void add_round_key(uchar state[4][4], const uchar round_key[16]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			state[j][i] ^= round_key[4 * i + j];
		}
	}
}

void sub_bytes(uchar state[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			state[j][i] = sbox[state[j][i]];
		}
	}
}

void inv_shift_rows(uchar state[4][4]) {
	uchar temp;
	// Row 1
	temp = state[1][3];
	state[1][3] = state[1][2];
	state[1][2] = state[1][1];
	state[1][1] = state[1][0];
	state[1][0] = temp;
	// Row 2
	temp = state[2][0];
	state[2][0] = state[2][2];
	state[2][2] = temp;
	temp = state[2][1];
	state[2][1] = state[2][3];
	state[2][3] = temp;
	// Row 3
	temp = state[3][0];
	state[3][0] = state[3][1];
	state[3][1] = state[3][2];
	state[3][2] = state[3][3];
	state[3][3] = temp;
}

void inv_mix_columns(uchar state[4][4]) {
	for (int c = 0; c < 4; c++) {
		uchar a[4];
		for (int i = 0; i < 4; i++) {
			a[i] = state[i][c];
		}
		state[0][c] = (uchar)(gf_mult(0x0e, a[0]) ^ gf_mult(0x0b, a[1]) ^ gf_mult(0x0d, a[2]) ^ gf_mult(0x09, a[3]));
		state[1][c] = (uchar)(gf_mult(0x09, a[0]) ^ gf_mult(0x0e, a[1]) ^ gf_mult(0x0b, a[2]) ^ gf_mult(0x0d, a[3]));
		state[2][c] = (uchar)(gf_mult(0x0d, a[0]) ^ gf_mult(0x09, a[1]) ^ gf_mult(0x0e, a[2]) ^ gf_mult(0x0b, a[3]));
		state[3][c] = (uchar)(gf_mult(0x0b, a[0]) ^ gf_mult(0x0d, a[1]) ^ gf_mult(0x09, a[2]) ^ gf_mult(0x0e, a[3]));
	}
}

void inv_sub_bytes(uchar state[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			state[j][i] = inv_sbox[state[j][i]];
		}
	}
}

void cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr) {
	uchar state[4][4];
	//afisare chei
	/*for (int i = 0; i < 11; i++) {
		printf("Key (%d):\n", i);
		print_roundkey((uchar*)(round_keys + i * 16));
		printf("\n");
	}*/
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			state[j][i] = in[i * 4 + j];
		}
	}
	add_round_key(state, round_keys);
	for (int round = 1; round < nr; round++) {
		sub_bytes(state);
		shift_rows(state);
		mix_columns(state);
		add_round_key(state, round_keys + round * 16);
	}
	sub_bytes(state);
	shift_rows(state);
	add_round_key(state, round_keys + nr * 16);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[i * 4 + j] = state[j][i];
		}
	}
}

void inv_cipher(const uchar in[16], uchar out[16], const uchar round_keys[], int nr) {
	uchar state[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			state[j][i] = in[i * 4 + j];
		}
	}
	add_round_key(state, round_keys + nr * 16);
	for (int round = nr - 1; round >= 1; round--) {
		inv_shift_rows(state);
		inv_sub_bytes(state);
		add_round_key(state, round_keys + round * 16);
		inv_mix_columns(state);
	}
	inv_shift_rows(state);
	inv_sub_bytes(state);
	add_round_key(state, round_keys);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out[i * 4 + j] = state[j][i];
		}
	}
}


void aes_128_encrypt(const uchar plaintext[16], const uchar key[16], uchar ciphertext[16]) {
	const int nk = 4;
	const int nr = 10;

	uchar round_keys[4 * (nr + 1) * 4];
	key_expansion(key, round_keys, nk, nr);

	cipher(plaintext, ciphertext, round_keys, nr);
}

void aes_128_decrypt(const uchar ciphertext[16], const uchar key[16], uchar plaintext[16]) {
	const int nk = 4;
	const int nr = 10;
	uchar round_keys[4 * (nr + 1) * 4];
	key_expansion(key, round_keys, nk, nr);
	inv_cipher(ciphertext, plaintext, round_keys, nr);
}