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
                                            jstring type,
                                            jint x, jint bytes) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *Pk_path = env->GetStringUTFChars(PK_PATH, nullptr);
    const char *CD1_path = env->GetStringUTFChars(CD1_PATH, nullptr);
    const char *Type = env->GetStringUTFChars(type, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" PK path : %s",Pk_path);
    LOGD(" CD1 path : %s",CD1_path);
    LOGD(" CD2 path : %s",Type);

    /*
    Note that scheme_type is now "bgv".
    */
    EncryptionParameters parms(scheme_type::bgv);
    size_t poly_modulus_degree = 16384;
    //size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(786433);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, bytes));
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
    BatchEncoder batch_encoder(context);

    const char* SIN = "sin";
    int result = strcmp(Type, SIN);
    if(result == 0){
        LOGD("encrypt the SIN");
        Plaintext plaintext;
        vector<int64_t> Req_matrix{x};
        batch_encoder.encode(Req_matrix, plaintext);
        // Encrypt the plaintext
        Ciphertext ciphertext;
        encryptor.encrypt(plaintext, ciphertext);

        std::ofstream file1(CD1_path, std::ios::binary);
        ciphertext.save(file1);
        file1.close();
    }
    else {

        // Convert number to binary
        std::vector<bool> bits;
        uint64_t temp = x;
        int size = 8;
        for (int i = 0; i < size; ++i) {
            bits.push_back(temp & 1);
            temp >>= 1;
        }
        //std::reverse(bits1.begin(), bits1.end());
        /*LOGD("\n----------------------Bytes----------------------------\n");
        LOGD("bit");
        for (bool bit: bits) {
            LOGD("%d", bit);
        }
        LOGD("\n----------------------Decryption----------------------------\n");*/

        // Create matrix of ciphertexts
        vector<Ciphertext> C;
        // Encrypt each bit
        for (size_t i = 0; i < bits.size(); ++i) {
            // Create plaintext for the bit
            Plaintext plaintext;
            vector<int64_t> Req_matrix{bits[i]};
            batch_encoder.encode(Req_matrix, plaintext);
            // Encrypt the plaintext
            Ciphertext ciphertext;
            encryptor.encrypt(plaintext, ciphertext);
            // Store the ciphertext in the matrix
            seal::Plaintext decrypted_result;
            C.emplace_back(ciphertext);
            decryptor.decrypt(ciphertext, decrypted_result);
            vector<int64_t> R;
            batch_encoder.decode(decrypted_result, R);
            //LOGD("%lld", R[0]);
        }

        std::ofstream file1(CD1_path, std::ios::binary);
        for (const auto &ciphertext: C) {
            ciphertext.save(file1);
        }
        file1.close();
    }

    std::ofstream file3(Sk_path, std::ios::binary);
    secret_key.save(file3);
    file3.close();

    std::ofstream file4(Pk_path, std::ios::binary);
    public_key.save(file4);
    file4.close();
    return 1;
}
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_seal_1bgv_1demo_MainActivity_Decrypt(JNIEnv *env, jobject thiz, jstring CD_PATH,jstring SK_PATH, jint bytes) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *CD_path = env->GetStringUTFChars(CD_PATH, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" CD path : %s",CD_path);

    EncryptionParameters parms(scheme_type::bgv);
    size_t poly_modulus_degree = 16384;
    //size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(786433);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, bytes));
    SEALContext context(parms);
    BatchEncoder batch_encoder(context);

    SecretKey secret_key;
    std::ifstream file1(Sk_path, std::ios::binary);
    secret_key.load(context,file1);
    file1.close();
    Decryptor decryptor(context, secret_key);

    Ciphertext x_squared;
    std::ifstream file2(CD_path, std::ios::binary);
    /*x_squared.load(context,file2);
    file2.close();

    Plaintext decrypted_result;
    decryptor.decrypt(x_squared, decrypted_result);
   // if(decrypted_result.is_zero()) LOGD("is zero");
    vector<int64_t> pod_result;
    batch_encoder.decode(decrypted_result, pod_result);
    LOGD("Resuuuuuuuuuuuult");

    return pod_result[0];*/

    // Read the encrypted results from the file
    // Save the current position of the cursor
    std::streampos currentPosition = file2.tellg();

    // Seek to the end of the file
    file2.seekg(0, std::ios::end);

    // Get the position of the cursor at the end of the file
    std::streampos endPosition = file2.tellg();

    // Reset the cursor back to the saved position
    file2.seekg(currentPosition);
    Ciphertext encrypted_result;
    jint proof[16];
    //while (currentPosition!=endPosition && encrypted_result.load(context,file2))
    for(int i=0;i<16;i++)
    {
        encrypted_result.load(context,file2);
        // Decrypt the ciphertext
        Plaintext plain_result;
        decryptor.decrypt(encrypted_result, plain_result);
        vector<int64_t> pod_result;
        batch_encoder.decode(plain_result, pod_result);
        proof[i] = pod_result[0];
        //proof.push_back(pod_result[0]);
        //LOGD("<%lld>",pod_result[0]);
        //currentPosition = file2.tellg();
    }
    file2.close();

    jintArray result = env->NewIntArray(16);
    // Set the values of the Java array
    env->SetIntArrayRegion(result, 0, 16, proof);

    return result;
}
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_seal_1bgv_1demo_MainActivity_vote_1Decrypt(JNIEnv *env, jobject thiz,
                                                       jstring CD_PATH, jstring SK_PATH) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *CD_path = env->GetStringUTFChars(CD_PATH, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" CD path : %s",CD_path);

    EncryptionParameters parms(scheme_type::bgv);
    //size_t poly_modulus_degree = 16384;
    size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(786433);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    SEALContext context(parms);
    BatchEncoder batch_encoder(context);

    SecretKey secret_key;
    std::ifstream file1(Sk_path, std::ios::binary);
    secret_key.load(context,file1);
    file1.close();
    Decryptor decryptor(context, secret_key);

    Ciphertext H_A;
    Ciphertext H_NULL;
    std::ifstream file2(CD_path, std::ios::binary);
    H_A.load(context,file2);
    H_NULL.load(context,file2);
    file2.close();

    Plaintext decrypted_result;
    decryptor.decrypt(H_A, decrypted_result);
   // if(decrypted_result.is_zero()) LOGD("is zero");
    vector<int64_t> pod_result;
    batch_encoder.decode(decrypted_result, pod_result);
    int proof[2];
    LOGD("r = %d",pod_result[0]);
    proof[0] = pod_result[0];
    decryptor.decrypt(H_NULL, decrypted_result);
    // if(decrypted_result.is_zero()) LOGD("is zero");
    batch_encoder.decode(decrypted_result, pod_result);
    proof[1] = pod_result[0];
    LOGD("NULL = %d",pod_result[0]);

    jintArray result = env->NewIntArray(2);
    // Set the values of the Java array
    env->SetIntArrayRegion(result, 0, 2, proof);

    return result;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_seal_1bgv_1demo_MainActivity_init_1vote(JNIEnv *env, jobject thiz,
                                                         jstring SK_PATH, jstring PK_PATH,
                                                         jstring CD1_PATH,
                                                         jint x, jint y,
                                                         jint z) {
    const char *Sk_path = env->GetStringUTFChars(SK_PATH, nullptr);
    const char *Pk_path = env->GetStringUTFChars(PK_PATH, nullptr);
    const char *CD_path = env->GetStringUTFChars(CD1_PATH, nullptr);

    LOGD(" SK path : %s",Sk_path);
    LOGD(" PK path : %s",Pk_path);
    LOGD(" CD1 path : %s",CD_path);

    /*
    Note that scheme_type is now "bgv".
    */
    EncryptionParameters parms(scheme_type::bgv);
    //size_t poly_modulus_degree = 16384;
    size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(786433);

    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    SEALContext context(parms);
    BatchEncoder batch_encoder(context);

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

    vector<int64_t> pod_matrix1{x};
    vector<int64_t> pod_matrix2{y};
    vector<int64_t> pod_matrix3{z};

    cout << "Input plaintext matrix:" << endl;
    Plaintext x_plain1;
    Plaintext x_plain2;
    Plaintext x_plain3;
    cout << "Encode plaintext to x_plain:" << endl;
    batch_encoder.encode(pod_matrix1, x_plain1);
    batch_encoder.encode(pod_matrix2, x_plain2);
    batch_encoder.encode(pod_matrix3, x_plain3);

    // Create the plaintext and ciphertext
    Ciphertext encrypted1;
    Ciphertext encrypted2;
    Ciphertext encrypted3;
    encryptor.encrypt(x_plain1, encrypted1);
    encryptor.encrypt(x_plain2, encrypted2);
    encryptor.encrypt(x_plain3, encrypted3);

    std::ofstream file1(CD_path, std::ios::binary);
    encrypted1.save(file1);
    encrypted2.save(file1);
    encrypted3.save(file1);
    file1.close();

    std::ofstream file3(Sk_path, std::ios::binary);
    secret_key.save(file3);
    file3.close();

    std::ofstream file4(Pk_path, std::ios::binary);
    public_key.save(file4);
    file4.close();

    //relin_keys.save(file5);

    return 1;
}
