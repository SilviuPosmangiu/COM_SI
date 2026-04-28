#include "aes_128_tests.h"

using namespace std;

void handleErrors()
{
    ERR_print_errors_fp(stderr);
    abort();
}

vector<unsigned char> aes128_encrypt_openssl(const vector<unsigned char>& plaintext, const vector<unsigned char>& key)
{
    if (key.size() != 16)
        throw runtime_error("Cheia trebuie sa aiba exact 16 bytes.");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        handleErrors();

    // Redimensionăm buffer-ul cat sa incapa textul initial si padding-ul maxim (16)
    vector<unsigned char> ciphertext(plaintext.size() + 16);
    int len = 0;
    int final_len = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key.data(), nullptr) != 1)
        handleErrors();

    if (EVP_CIPHER_CTX_set_padding(ctx, 1) != 1)
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
    cout << "}" << dec << '\n';
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

bool compare_arrays(const uchar arr1[], const uchar arr2[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr1[i] != arr2[i])
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

void test_aes_128() {
    vector<unsigned char> plaintext = {
        0x32, 0x43, 0xf6, 0xa8,
        0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2,
        0xe0, 0x37, 0x07, 0x34
    };

    vector<unsigned char> key = {
        0x2b, 0x7e, 0x15, 0x16,
        0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88,
        0x09, 0xcf, 0x4f, 0x3c
    };

    uchar plaintext_mine[32] = { 0 };
    uchar key_mine[16];
    uchar ciphertext_mine[32] = { 0 };
    uchar decrypted_mine[32] = { 0 };

    for (int i = 0; i < 16; i++)
    {
        plaintext_mine[i] = plaintext[i];
        key_mine[i] = key[i];
    }

    initialize_sbox();

    printf("\n--- TEST SINGLE BLOCK ---\n");
    printf("\nCryptare AES-128:\n\n");
    printf("Plaintext: ");
    print_hex(plaintext);
    printf("Cheie: ");
    print_hex(key);
    printf("\n");

    auto ref = aes128_encrypt_openssl(plaintext, key);

    cout << "Rezultatul OpenSSL: ";
    print_hex(ref);

    int cipher_len = 0;
    aes_128_encrypt(plaintext_mine, 16, key_mine, ciphertext_mine, &cipher_len);

    cout << "Rezultatul functiei implementate: ";
    print_hex(ciphertext_mine, cipher_len);

    if (compare_array_with_vector(ciphertext_mine, ref, cipher_len))
        cout << "[SUCCESS] Criptarea a fost corecta\n";
    else
        cout << "[FAILED] Criptarea NU a fost corecta\n";

    printf("\nDecriptare AES-128:\n\n");
    printf("Ciphertext: ");
    print_hex(ciphertext_mine, cipher_len);
    printf("Cheie: ");
    print_hex(key);
    printf("\n");

    int decrypted_len = 0;
    aes_128_decrypt(ciphertext_mine, cipher_len, key_mine, decrypted_mine, &decrypted_len);

    printf("Plaintext decrypted: ");
    print_hex(decrypted_mine, decrypted_len);

    if (compare_arrays(plaintext_mine, decrypted_mine, 16) && decrypted_len == 16)
        cout << "[SUCCESS] Decriptarea a fost corecta\n";
    else
        cout << "[FAILED] Decriptarea NU a fost corecta\n";
	cout << "---------------------------------------------\n\n";

    printf("\n--- TEST MULTI BLOCK (cu padding) ---\n");
    
    // Un mesaj de 37 octeți, ceea ce necesită cel puțin 3 blocuri și padding
    vector<unsigned char> multi_plaintext = {
        'A', 'c', 'e', 's', 't', 'a', ' ', 'e', 's', 't', 'e', ' ',
        'u', 'n', ' ', 'm', 'e', 's', 'a', 'j', ' ', 'm', 'a', 'i', ' ',
        'l', 'u', 'n', 'g', ' ', 'p', 'e', 'n', 't', 'r', 'u', '!'
    };

    uchar multi_pt_mine[128] = { 0 };
    uchar multi_ct_mine[128] = { 0 };
    uchar multi_dt_mine[128] = { 0 };

    for (size_t i = 0; i < multi_plaintext.size(); i++)
    {
        multi_pt_mine[i] = multi_plaintext[i];
    }

    auto multi_ref = aes128_encrypt_openssl(multi_plaintext, key);

    printf("Plaintext multi-bloc: ");
    print_hex(multi_plaintext);
    cout << "\nRezultat OpenSSL: ";
    print_hex(multi_ref);

    int multi_cipher_len = 0;
    aes_128_encrypt(multi_pt_mine, static_cast<int>(multi_plaintext.size()), key_mine, multi_ct_mine, &multi_cipher_len);

    cout << "Rezultat Criptare Implementata: ";
    print_hex(multi_ct_mine, multi_cipher_len);

    if (compare_array_with_vector(multi_ct_mine, multi_ref, multi_cipher_len))
        cout << "[SUCCESS] Criptarea multi-bloc coincida!\n";
    else
        cout << "[FAILED] Criptarea multi-bloc esueaza!\n";

    int multi_decrypted_len = 0;
    aes_128_decrypt(multi_ct_mine, multi_cipher_len, key_mine, multi_dt_mine, &multi_decrypted_len);

    printf("Plaintext Decriptat Implementat: ");
    print_hex(multi_dt_mine, multi_decrypted_len);

    if (compare_arrays(multi_pt_mine, multi_dt_mine, multi_decrypted_len) && multi_decrypted_len == multi_plaintext.size())
        cout << "[SUCCESS] Decriptarea multi-bloc a functionat!\n";
    else
        cout << "[FAILED] Decriptarea multi-bloc esueaza!\n";
        
    cout << "---------------------------------------------\n\n";
}