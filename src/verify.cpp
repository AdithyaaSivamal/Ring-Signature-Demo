#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include "util.h"  

using namespace std;

int main() {
    // header
    cout << "\n========================================\n";
    cout << "      Ring Signature Verifier           \n";
    cout << "========================================\n\n";
    cout << "Welcome! This tool verifies a ring signature.\n";
    cout << "Ensure 'publickey.txt', 'message.txt', and 'signature.txt' are in the /data folder.\n\n";

    // reading 
    cout << "[1/3] Loading public keys from 'publickey.txt'... ";
    vector<string> publicKeys = readLines("../data/publickey.txt", 4);
    cout << "Done!\n";

    cout << "[2/3] Loading message from 'message.txt'... ";
    string message = readFile("../data/message.txt");
    cout << "Done!\n";

    cout << "[3/3] Loading signature from 'signature.txt'... ";
    string signature = readFile("../data/signature.txt");
    cout << "Done!\n\n";

    // Parse signature to components
    cout << "Parsing signature... ";
    vector<string> sigParts = split(signature, '\n');
    if (sigParts.size() < 4) {
        cout << "\nError: Signature file is malformed!\n";
        cout << "Result: False\n";
        cout << "========================================\n";
        return 1;
    }
    string v_str = sigParts[0];
    string y1_str = sigParts[1];
    string y2_str = sigParts[2];
    string keyStr = sigParts[3];
    cout << "Done!\n";

    // Initialize OpenSSL BIGNUMs
    BIGNUM *e1 = BN_new(), *n1 = BN_new(), *e2 = BN_new(), *n2 = BN_new();
    BIGNUM *v = BN_new(), *y1 = BN_new(), *y2 = BN_new();
    BN_dec2bn(&e1, publicKeys[0].c_str());
    BN_dec2bn(&n1, publicKeys[1].c_str());
    BN_dec2bn(&e2, publicKeys[2].c_str());
    BN_dec2bn(&n2, publicKeys[3].c_str());
    BN_dec2bn(&v, v_str.c_str());
    BN_dec2bn(&y1, y1_str.c_str());
    BN_dec2bn(&y2, y2_str.c_str());

    // Setup AES encryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, (unsigned char*)keyStr.c_str(), NULL);

    // Encryption
    string paddedMsg = message + string(16 - message.length() % 16, '\0');
    vector<unsigned char> encrypted(paddedMsg.length() + 16);
    int len, ciphertext_len;
    EVP_EncryptUpdate(ctx, encrypted.data(), &len, (unsigned char*)paddedMsg.c_str(), paddedMsg.length());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len);
    ciphertext_len += len;
    encrypted.resize(ciphertext_len);

    BIGNUM *encryptedBN = BN_bin2bn(encrypted.data(), encrypted.size(), NULL);
    BIGNUM *xored = BN_new();
    BN_CTX *bn_ctx = BN_CTX_new();
    bool valid = false;

    // Verify signatures
    cout << "Verifying signature... ";
    for (int signer = 1; signer <= 2; signer++) {
        BIGNUM *computed_y1 = BN_new(), *computed_y2 = BN_new();
        
        BN_mod_exp(computed_y1, signer == 1 ? e2 : e1, v, signer == 1 ? n2 : n1, bn_ctx);
        BN_mod_add(xored, v, encryptedBN, signer == 1 ? n1 : n2, bn_ctx);
        BN_mod_exp(computed_y2, signer == 1 ? e1 : e2, xored, signer == 1 ? n1 : n2, bn_ctx);

        if (BN_cmp(computed_y1, y1) == 0 && BN_cmp(computed_y2, y2) == 0) {
            valid = true;
            break;
        }
        
        BN_free(computed_y1);
        BN_free(computed_y2);
    }
    cout << "Done!\n\n";

    // Display results
    cout << "========================================\n";
    cout << "Verification Result: " << (valid ? "True" : "False") << "\n";
    cout << "========================================\n";

    // Cleanup
    BN_free(e1); BN_free(n1); BN_free(e2); BN_free(n2);
    BN_free(v); BN_free(y1); BN_free(y2); BN_free(encryptedBN); BN_free(xored);
    BN_CTX_free(bn_ctx);
    EVP_CIPHER_CTX_free(ctx);

    return 0;
}
