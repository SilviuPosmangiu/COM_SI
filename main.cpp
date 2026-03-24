#include <iostream>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "functions.h"

using namespace std;

void handleErrors()
{
    ERR_print_errors_fp(stderr);
    abort();
}

vector<unsigned char> aes128_encrypt_openssl_no_padding(const vector<unsigned char>& plaintext,const vector<unsigned char>& key)
{
    if (plaintext.size() != 16)
        throw runtime_error("Plaintext-ul trebuie sa aiba exact 16 bytes.");

    if (key.size() != 16)
        throw runtime_error("Cheia trebuie sa aiba exact 16 bytes.");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        handleErrors();

    vector<unsigned char> ciphertext(16);
    int len = 0;
    int final_len = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key.data(), nullptr) != 1)
        handleErrors();

    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1)
        handleErrors();

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size())) != 1)
    {
        handleErrors();
    }

    final_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
        handleErrors();

    final_len += len;
    ciphertext.resize(final_len);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

void print_hex(const vector<unsigned char>& data)
{
	cout << "{ ";
    for (unsigned char c : data)
    {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(c) << " ";
    }
    cout << dec << '\n';
}

bool compare_array_with_vector(const uchar arr[], const vector<unsigned char>& vec, int size)
{
    if (vec.size() != static_cast<size_t>(size))
        return false;

    for (int i = 0; i < size; i++)
    {
        if (arr[i] != vec[i])
            return false;
    }

    return true;
}

void print_hex(const uchar* data, size_t length) {
    printf("{ ");
    for (size_t i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("}\n");
}

int main()
{
    vector<unsigned char> plaintext = {
        '1','2','3','4',
        '5','6','7','8',
        '9','0','a','b',
        'c','d','e','f'
    };

    vector<unsigned char> key = {
        0x2b, 0x7e, 0x15, 0x16,
        0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88,
        0x09, 0xcf, 0x4f, 0x3c
    };

    auto ref = aes128_encrypt_openssl_no_padding(plaintext, key);

	initialize_sbox();
    print_sbox();

    cout << "Rezultatul OpenSSL: ";
    print_hex(ref);

    uchar plaintext_mine[16];
    uchar key_mine[16];
    uchar ciphertext_mine[16] = { 0 };

    for (int i = 0; i < 16; i++)
    {
        plaintext_mine[i] = plaintext[i];
        key_mine[i] = key[i];
    }

    aes_128_encrypt(plaintext_mine, key_mine, ciphertext_mine);

    cout << "Rezultatul functiei implementate: ";
    print_hex(ciphertext_mine, 16);

    if (compare_array_with_vector(ciphertext_mine, ref, 16))
        cout << "Rezultatele coincid\n";
    else
        cout << "Rezultatele NU coincid\n";

    return 0;
}

