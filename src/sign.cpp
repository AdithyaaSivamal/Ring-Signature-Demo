#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include "util.h"  

using namespace std;

int main() {
    // Fancy header
    cout << "\n========================================\n";
    cout << "      Ring Signature Generator          \n";
    cout << "========================================\n\n";
    cout << "Welcome! This tool generates a ring signature.\n";
    cout << "Ensure 'publickey.txt' and 'message.txt' are ready.\n\n";

    // Load public keys 
    cout << "[1/2] Loading public keys from 'publickey.txt'... ";
    vector<string> publicKeys = readLines("../data/publickey.txt", 4);
    if (publicKeys.size() != 4) {
        cout << "\nError: 'publickey.txt' must have exactly 4 lines (e1, n1, e2, n2)!\n";
        cout << "========================================\n";
        return 1;
    }
    cout << "Done!\n";

    // Load message
    cout << "[2/2] Loading message from 'message.txt'... ";
    string message = readFile("../data/message.txt");
    if (message.empty()) {
        cout << "\nError: 'message.txt' is empty!\n";
        cout << "========================================\n";
        return 1;
    }
    cout << "Done!\n\n";

    // Get signer input
    int signer;
    cout << "Enter signer (1 or 2): ";
    cin >> signer;
    if (signer != 1 && signer != 2) {
        cout << "\nError: Invalid signer choice! Must be 1 or 2.\n";
        cout << "========================================\n";
        return 1;
    }

    // Get private key
    string privateKey;
    cout << "Enter private key (d) for user " << signer << ": ";
    cin >> privateKey;

    // Initialize OpenSSL BIGNUMs
    BIGNUM *e1 = BN_new(), *n1 = BN_new(), *e2 = BN_new(), *n2 = BN_new();
    BIGNUM *d = BN_new();
    BN_dec2bn(&e1, publicKeys[0].c_str());
    BN_dec2bn(&n1, publicKeys[1].c_str());
    BN_dec2bn(&e2, publicKeys[2].c_str());
    BN_dec2bn(&n2, publicKeys[3].c_str());
    BN_dec2bn(&d, privateKey.c_str());

    // Generate AES key
    unsigned char key[16];
    RAND_bytes(key, 16);

    // Encrypt message with AES
    cout << "Generating signature... ";
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
    
    string paddedMsg = message + string(16 - message.length() % 16, '\0');
    vector<unsigned char> encrypted(paddedMsg.length() + 16);
    int len, ciphertext_len;
    EVP_EncryptUpdate(ctx, encrypted.data(), &len, (unsigned char*)paddedMsg.c_str(), paddedMsg.length());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len);
    ciphertext_len += len;
    encrypted.resize(ciphertext_len);

    // Generate ring signature components
    BIGNUM *v = BN_new();
    BN_rand_range(v, n1);

    BIGNUM *y1 = BN_new(), *y2 = BN_new();
    BN_CTX *bn_ctx = BN_CTX_new();

    int nonSigner = (signer == 1) ? 2 : 1;
    BN_mod_exp(y1, signer == 1 ? e2 : e1, v, signer == 1 ? n2 : n1, bn_ctx);

    BIGNUM *encryptedBN = BN_bin2bn(encrypted.data(), encrypted.size(), NULL);
    BIGNUM *xored = BN_new();
    BN_mod_add(xored, v, encryptedBN, signer == 1 ? n1 : n2, bn_ctx);
    BN_mod_exp(y2, signer == 1 ? e1 : e2, xored, signer == 1 ? n1 : n2, bn_ctx);

    // Construct signature
    char *v_str = BN_bn2dec(v);
    char *y1_str = BN_bn2dec(y1);
    char *y2_str = BN_bn2dec(y2);
    string signature = string(v_str) + "\n" + string(y1_str) + "\n" + string(y2_str) + "\n" + string((char*)key, 16);

    // Write to file
    writeFile("../data/signature.txt", signature);
    cout << "Done!\n\n";

    // Success message
    cout << "========================================\n";
    cout << "Signature generated successfully!\n";
    cout << "Output written to '/data/signature.txt'.\n";
    cout << "========================================\n";

    // Cleanup
    BN_free(e1); BN_free(n1); BN_free(e2); BN_free(n2); BN_free(d);
    BN_free(v); BN_free(y1); BN_free(y2); BN_free(encryptedBN); BN_free(xored);
    BN_CTX_free(bn_ctx);
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_free(v_str); OPENSSL_free(y1_str); OPENSSL_free(y2_str);

    return 0;
}
