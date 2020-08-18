//
// Created by markus on 16/01/2020.
// Source: https://gist.github.com/irbull/08339ddcd5686f509e9826964b17bb59
//

#include "verifyFile.hpp"

#ifdef AUTOBET_BUILD_UPDATER
#include <iostream>
#include <utility>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <fstream>

#include "../logger.hpp"

const char *publicKey = "-----BEGIN PUBLIC KEY-----\n"
                        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAs1rhLJYbIeLXsRldMp1I\n"
                        "UYLwDdda5jmG/Fg0PT9opeyb1/llFNhrRRdpjEw4aG69kAyttg8sQT8mXl1hMs8/\n"
                        "rsILqU7RIveAkbMXcn5Jsk15vpyY8eTc1QGMUtLM/DUx8GAtFQlWZCw+7Uvx+mRX\n"
                        "WT2lWitqAc0z7L93LDkwBa2YgkmcTeZ0+tR1QCyiBjzV4U0k4qz8FMG2Js5kpgwl\n"
                        "Z4usp+mjnrJwZt+zgZ0vBDYBho7KONhI0IadRvkFkycNB1I4fnQwvO1QaA/FDaiX\n"
                        "ycdwx7kp4K/srEuB8IeFt7mp3/lLk53E2xOZ3YAjHgBk3RPd5kE+U0AzP3ZAZUAS\n"
                        "wQIDAQAB\n"
                        "-----END PUBLIC KEY-----";

RSA *createPrivateRSA(const std::string &key) {
    RSA *rsa = nullptr;
    const char *c_string = key.c_str();
    BIO *keybio = BIO_new_mem_buf((void *) c_string, -1);
    if (keybio == nullptr) {
        return nullptr;
    }
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, nullptr, nullptr);
    return rsa;
}

RSA *createPublicRSA(const std::string &key) {
    RSA *rsa = nullptr;
    BIO *keybio;
    const char *c_string = key.c_str();
    keybio = BIO_new_mem_buf((void *) c_string, -1);
    if (keybio == nullptr) {
        return nullptr;
    }
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, nullptr, nullptr);
    return rsa;
}

bool RSASign(RSA *rsa, const unsigned char *Msg, size_t MsgLen, unsigned char **EncMsg, size_t *MsgLenEnc) {
    EVP_MD_CTX *m_RSASignCtx = EVP_MD_CTX_create();
    EVP_PKEY *priKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(priKey, rsa);
    if (EVP_DigestSignInit(m_RSASignCtx, nullptr, EVP_sha256(), nullptr, priKey) <= 0) {
        return false;
    }
    if (EVP_DigestSignUpdate(m_RSASignCtx, Msg, MsgLen) <= 0) {
        return false;
    }
    if (EVP_DigestSignFinal(m_RSASignCtx, nullptr, MsgLenEnc) <= 0) {
        return false;
    }
    *EncMsg = (unsigned char *) malloc(*MsgLenEnc);
    if (EVP_DigestSignFinal(m_RSASignCtx, *EncMsg, MsgLenEnc) <= 0) {
        return false;
    }
    EVP_MD_CTX_destroy(m_RSASignCtx);
    return true;
}

bool RSAVerifySignature(RSA *rsa, unsigned char *MsgHash, size_t MsgHashLen, const char *Msg, size_t MsgLen,
                        bool *Authentic) {
    *Authentic = false;
    EVP_PKEY *pubKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pubKey, rsa);
    EVP_MD_CTX *m_RSAVerifyCtx = EVP_MD_CTX_create();

    if (EVP_DigestVerifyInit(m_RSAVerifyCtx, nullptr, EVP_sha256(), nullptr, pubKey) <= 0) {
        return false;
    }
    if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, Msg, MsgLen) <= 0) {
        return false;
    }
    int AuthStatus = EVP_DigestVerifyFinal(m_RSAVerifyCtx, MsgHash, MsgHashLen);
    if (AuthStatus == 1) {
        *Authentic = true;
        EVP_MD_CTX_destroy(m_RSAVerifyCtx);
        return true;
    } else {
        *Authentic = false;
        EVP_MD_CTX_destroy(m_RSAVerifyCtx);
        return AuthStatus == 0;
    }
}

void Base64Encode(const unsigned char *buffer,
                  size_t length,
                  char **base64Text) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, buffer, (int) length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    *base64Text = (*bufferPtr).data;
}

size_t calcDecodeLength(const char *b64input) {
    size_t len = strlen(b64input), padding = 0;

    if (b64input[len - 1] == '=' && b64input[len - 2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len - 1] == '=') //last char is =
        padding = 1;
    return (len * 3) / 4 - padding;
}

void Base64Decode(const char *b64message, unsigned char **buffer, size_t *length) {
    BIO *bio, *b64;

    int decodeLen = (int) calcDecodeLength(b64message);
    *buffer = (unsigned char *) malloc(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    *length = BIO_read(bio, *buffer, (int) strlen(b64message));
    BIO_free_all(bio);
}
#endif

void fileCrypt::signInstaller() {
#ifdef AUTOBET_BUILD_UPDATER
    const char *dig = fileCrypt::signMessage("private.pem", "autobet_installer.exe");
    if (dig)
        fileCrypt::writeToFile("autobet_installer.pem", dig);
#endif
}

void fileCrypt::writeToFile(const std::string &path, const char *toWrite) {
#ifdef AUTOBET_BUILD_UPDATER
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        logger::StaticLogger::error("Unable to write to file: " + path);
        return;
    }

    stream.write(toWrite, strlen(toWrite));
    stream.flush();
    stream.close();
#endif
}

std::string fileCrypt::getFileContent(const std::string &path) {
#ifdef AUTOBET_BUILD_UPDATER
    std::ifstream file(path);
    if (!file.is_open()) {
        logger::StaticLogger::error("Unable to read file: " + path);
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
#else
    return "";
#endif
}

char *fileCrypt::signMessage(const std::string &privateKeyPath, const std::string &plainTextPath) {
#ifdef AUTOBET_BUILD_UPDATER
    std::string privateKey = getFileContent(privateKeyPath);
    if (privateKey.empty()) return nullptr;
    RSA *privateRSA = createPrivateRSA(privateKey);
    unsigned char *encMessage;
    char *base64Text;
    size_t encMessageLength;
    std::string plainText = getFileContent(plainTextPath);
    if (plainText.empty()) return nullptr;
    RSASign(privateRSA, (unsigned char *) plainText.c_str(), plainText.length(), &encMessage, &encMessageLength);
    Base64Encode(encMessage, encMessageLength, &base64Text);
    free(encMessage);
    return base64Text;
#else
    return nullptr;
#endif
}

bool fileCrypt::verifySignature(const std::string &plainTextPath, const std::string &signatureBase64) {
#ifdef AUTOBET_BUILD_UPDATER
    RSA *publicRSA = createPublicRSA(publicKey);
    unsigned char *encMessage;
    size_t encMessageLength;
    bool authentic;
    Base64Decode(signatureBase64.c_str(), &encMessage, &encMessageLength);
    std::string plainText = getFileContent(plainTextPath);
    if (plainText.empty()) return false;
    bool result = RSAVerifySignature(publicRSA, encMessage, encMessageLength, plainText.c_str(), plainText.length(),
                                     &authentic);
    return result & authentic;
#else
    return false;
#endif
}