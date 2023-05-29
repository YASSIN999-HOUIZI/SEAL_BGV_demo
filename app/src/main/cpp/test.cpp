#include <jni.h>
#include <iostream>
#include <fstream>
#include "seal/seal.h"
#include <android/log.h>

#define  LOG_TAG    "TFHE :"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
using namespace std;
using namespace seal;
//
// Created by yassine on 25‏/5‏/2023.
//

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_seal_1bgv_1demo_MainActivity_init(JNIEnv *env, jobject thiz,
                                            jstring SK_PATH,jstring PK_PATH,jstring CD1_PATH,
                                            jstring CD2_PATH,
                                            jint x, jint y) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *Pk_path = env->GetStringUTFChars(PK_PATH, nullptr);
    const char *CD1_path = env->GetStringUTFChars(CD1_PATH, nullptr);
    const char *CD2_path = env->GetStringUTFChars(CD2_PATH, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" PK path : %s",Pk_path);
    LOGD(" CD1 path : %s",CD1_path);
    LOGD(" CD2 path : %s",CD2_path);


    /*
    Note that scheme_type is now "bgv".
    */
    EncryptionParameters parms(scheme_type::bgv);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    SEALContext context(parms);


    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    /*
    Batching and slot operations are the same in BFV and BGV.
    */
    BatchEncoder batch_encoder(context);

    vector<uint64_t> pod_matrix1{static_cast<unsigned long long>(x)};
    vector<uint64_t> pod_matrix2{static_cast<unsigned long long>(y)};

    cout << "Input plaintext matrix:" << endl;
    Plaintext x_plain1;
    Plaintext x_plain2;
    cout << "Encode plaintext to x_plain:" << endl;
    batch_encoder.encode(pod_matrix1, x_plain1);
    batch_encoder.encode(pod_matrix2, x_plain2);

    // Create the plaintext and ciphertext
    Ciphertext encrypted1;
    Ciphertext encrypted2;
    encryptor.encrypt(x_plain1, encrypted1);
    encryptor.encrypt(x_plain2, encrypted2);

    std::ofstream file1(CD1_path, std::ios::binary);
    encrypted1.save(file1);
    file1.close();

    std::ofstream file2(CD2_path, std::ios::binary);
    encrypted2.save(file2);
    file2.close();

    std::ofstream file3(Sk_path, std::ios::binary);
    secret_key.save(file3);
    file3.close();

    std::ofstream file4(Pk_path, std::ios::binary);
    public_key.save(file4);
    file4.close();
    /*
    Then we compute x^2.
    */
    /*Ciphertext x_squared;
    evaluator.multiply(encrypted1,encrypted2,x_squared);
    //cout << "    + size of x_squared: " << x_squared.size() << endl;
    Plaintext decrypted_result;
    decryptor.decrypt(x_squared, decrypted_result);
    //cout << "Result ====================== " << decrypted_result.to_string() << endl;
    vector<uint64_t> pod_result;
    uint64_t pod_result1;
    batch_encoder.decode(decrypted_result, pod_result);
    cout << "[+] result from the " << pod_result[0] << endl;

    return pod_result[0];
    */
    return 1;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_seal_1bgv_1demo_MainActivity_Decrypt(JNIEnv *env, jobject thiz, jstring CD_PATH,jstring SK_PATH) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *CD_path = env->GetStringUTFChars(CD_PATH, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" CD path : %s",CD_path);

    EncryptionParameters parms(scheme_type::bgv);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    SEALContext context(parms);
    BatchEncoder batch_encoder(context);

    SecretKey secret_key;
    std::ifstream file1(Sk_path, std::ios::binary);
    secret_key.load(context,file1);
    file1.close();
    Decryptor decryptor(context, secret_key);

    Ciphertext x_squared;
    std::ifstream file2(CD_path, std::ios::binary);
    x_squared.load(context,file2);
    file2.close();

    Plaintext decrypted_result;
    decryptor.decrypt(x_squared, decrypted_result);
    vector<uint64_t> pod_result;
    batch_encoder.decode(decrypted_result, pod_result);
    cout << "[+] result from the " << pod_result[0] << endl;

    return pod_result[0];
}