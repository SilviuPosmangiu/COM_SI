#include "aes_128_tests.h"
#include "aes_128.h"
#include "communications.h"



int main(int argc, char* argv[]) {
    test_aes_128();
    return start_communication("127.0.0.1");
}

