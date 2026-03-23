#include <iostream>
#include "functions.h"

using namespace std;

void main() {
	const char* plaintext = "Hello, World!";
	const char* key = "mysecretkey";
	
	cout << "COM_SI\n\n";

	initialize_sbox();
	print_sbox();

	cout << "Plaintext: " << plaintext << endl;
	cout << "Key: " << key << endl;
	//cout << "Ciphertext: " << aes_128_encrypt((const unsigned char*)plaintext, (const unsigned char*)key) << endl;
	
}