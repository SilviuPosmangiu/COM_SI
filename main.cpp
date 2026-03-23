#include <iostream>
#include "functions.h"

using namespace std;

void print_hex(const uchar* data, size_t length) {
	printf("{ ");
	for (size_t i = 0; i < length; i++) {
		printf("%02x ", data[i]);
	}
	printf("}\n");
}

void main() {
	const uchar plaintext[16] = {
		0x32, 0x43, 0xf6, 0xa8,
		0x88, 0x5a, 0x30, 0x8d,
		0x31, 0x31, 0x98, 0xa2,
		0xe0, 0x37, 0x07, 0x34
	};
	const uchar key[16] = {
		0x2b, 0x7e, 0x15, 0x16,
		0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88,
		0x09, 0xcf, 0x4f, 0x3c
	};

	uchar block[16];
	uchar ciphertext[16];
	
	cout << "COM_SI\n\n";

	initialize_sbox();
	print_sbox();

	aes_128_encrypt(plaintext, key, ciphertext);

	cout << "\nAES-128 Encryption:\n";
	cout << "Plaintext: ";
	print_hex(plaintext, 16);
	cout << "Key: ";
	print_hex(key, 16);
	cout << "Ciphertext: ";
	print_hex(ciphertext, 16);

	cout << "\nAES-128 Decryption:\n";
	uchar decrypted[16];
	aes_128_decrypt(ciphertext, key, decrypted);

	cout << "Ciphertext: ";
	print_hex(ciphertext, 16);
	cout << "Key: ";
	print_hex(key, 16);
	cout << "Decrypted: ";
	print_hex(decrypted, 16);

	
}