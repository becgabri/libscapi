#pragma once
#include "AsymmetricEnc.hpp"
#include "../primitives/TrapdoorPermutations.hpp"
#include "../infra/MathAlgorithms.hpp"

/**
* This class represents a Public Key suitable for the Damgard-Jurik Encryption Scheme. Although the constructor is public, it should only be instantiated by the
* Encryption Scheme itself via the generateKey function.
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Yael Ejgenberg)
*
*/
class DamgardJurikPublicKey : public PublicKey, public KeySendableData {

private:

	biginteger modulus;
public:
	DamgardJurikPublicKey(biginteger modulus) {	this->modulus = modulus; }
	
	string getAlgorithm() override { return "DamgardJurik"; }

	vector<byte> getEncoded() override { 
		int size = bytesCount(modulus);
		byte* num = new byte[size];
		encodeBigInteger(modulus, num, size);
		vector<byte> out;
		copy_byte_array_to_byte_vector(num, size, out, 0);
		return out;
	}
	
	biginteger getModulus() { return modulus;	}

	/**
	* This function is used when an Damgard Jurik Public Key needs to be sent via a {@link edu.biu.scapi.comm.Channel} or any other means of sending data (including serialization).
	* It retrieves all the data needed to reconstruct this Public Key at a later time and/or in a different VM.
	* It puts all the data in an instance of the relevant class that implements the KeySendableData interface.
	* In order to deserialize this into a DamgardJurikPublicKey all you need to do is cast the serialized object with (DamgardJurikPublicKey)
	* @return the KeySendableData object
	*/
	shared_ptr<KeySendableData> generateSendableData() {
		//Since DamgardJurikPublicKey is both a PublicKey and a KeySendableData, on the one hand it has to implement
		//the generateSendableData() function, but on the other hand it is in itself an KeySendableData, so we do not really
		//generate sendable data, but just return this object.
		return shared_ptr<KeySendableData>(this);
	}

	string toString() override;

	void initFromString(const string & row) override;
};

/**
* This class represents a Private Key suitable for the Damgard-Jurik Encryption Scheme. Although the constructor is  public, it should only be instantiated by the
* Encryption Scheme itself via the generateKey function.
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Yael Ejgenberg)
*
*/
class DamgardJurikPrivateKey : public PrivateKey, KeySendableData {

private:
	biginteger t;
	biginteger dForS1; //Pre-calculated d in the case the s == 1
	biginteger p;
	biginteger q;

	/**
	* This function generates a value d such that d = 1 mod N and d = 0 mod t, using the Chinese Remainder Theorem.
	*/
	biginteger generateD(biginteger N, biginteger t); 

public:
	DamgardJurikPrivateKey(RSAModulus rsaMod);

	string getAlgorithm() override { return "DamgardJurik"; }

	vector<byte> getEncoded() override { throw NotImplementedException("");	}

	biginteger getT() {	return t; }

	biginteger getDForS1() { return dForS1;	}

	biginteger getP() {	return p; }

	biginteger getQ() {	return q; }

	string toString() override;

	void initFromString(const string & row) override;
};

/**
* Parameters for DamgardJurik key generation based on RSA modulus.<p>
* These parameters will be used to generate a Key Pair for Damgard Jurik based on RSA modulus n such that n = p*q of length k bits.
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Yael Ejgenberg)
*
*/
class DJKeyGenParameterSpec : public AlgorithmParameterSpec {

private:
	int modulusLength;
	int certainty;

	/**
	* Default constructor. The values of the RSA modulus length and the certainty are chosen by SCAPI
	*/
public:
	
	/**
	* Constructor that lets you set the length of the RSA modulus and the certainty required regarding the primeness of p and q.
	*
	* @param modulusLength
	* @param certainty
	*/
	DJKeyGenParameterSpec(int modulusLength = 1024, int certainty = 40) {
		this->modulusLength = modulusLength;
		this->certainty = certainty;
	}

	int getModulusLength() { return modulusLength; }

	int getCertainty() { return certainty; }
};

/**
* Damgard Jurik is an asymmetric encryption scheme based on the Paillier encryption scheme.
* This encryption scheme is CPA-secure and Indistinguishable.
*
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Yael Ejgenberg)
*
*/
class DamgardJurikEnc : public AsymAdditiveHomomorphicEnc {

private:
	shared_ptr<DamgardJurikPublicKey> publicKey;
	shared_ptr<DamgardJurikPrivateKey> privateKey;
	mt19937 random;
	bool keySet;

	int consts = -1;

	/**
	* This function generates a value d such that d = 1 mod N and d = 0 mod t, using the Chinese Remainder Theorem.
	*/
	biginteger generateD(biginteger N, biginteger t);

public:
	/**
	* Constructor that lets the user choose the source of randomness.
	* @param rnd source of randomness.
	*/
	DamgardJurikEnc() { random = get_seeded_random(); }

	/**
	* Initializes this DamgardJurik encryption scheme with (public, private) key pair.
	* After this initialization the user can encrypt and decrypt messages.
	* @param publicKey should be DamgardJurikPublicKey.
	* @param privateKey should be DamgardJurikPrivateKey.
	* @throws InvalidKeyException if the given keys are not instances of DamgardJurik keys.
	*/
	void setKey(shared_ptr<PublicKey> publicKey, shared_ptr<PrivateKey> privateKey) override;

	/**
	* Initializes this DamgardJurik encryption scheme with public key.
	* Setting only the public key the user can encrypt messages but can not decrypt messages.
	* @param publicKey should be DamgardJurikPublicKey
	* @throws InvalidKeyException if the given key is not instance of DamgardJurikPublicKey.
	*/
	void setKey(shared_ptr<PublicKey> publicKey) override {	setKey(publicKey, NULL); }

	bool isKeySet() override { return keySet; }

	/**
	* Returns the PublicKey of this DamgardJurik encryption scheme.
	* This function should not be use to check if the key has been set.
	* To check if the key has been set use isKeySet function.
	* @return the DamgardJurikPublicKey
	* @throws IllegalStateException if no public key was set.
	*/
	shared_ptr<PublicKey> getPublicKey() override;

	/**
	* @return the name of this AsymmetricEnc - DamgardJurik.
	*/
	string getAlgorithmName() override { return "DamgardJurik";	}

	/**
	* DamgardJurik encryption scheme has no limit of the byte array length to generate a plaintext from.
	* @return false.
	*/
	bool hasMaxByteArrayLengthForPlaintext() override { return false; }

	/**
	* DamgardJurik encryption can get any plaintext length.
	* @throws NoMaxException
	*/
	int getMaxLengthOfByteArrayForPlaintext() override {
		throw runtime_error("DamgardJurik encryption can get any plaintext length");
	}

	/**
	* Generates a Plaintext suitable to DamgardJurik encryption scheme from the given message.
	* @param msg byte array to convert to a Plaintext object.
	*/
	shared_ptr<Plaintext> generatePlaintext(vector<byte> text) override {
		return make_shared<BigIntegerPlainText>(decodeBigInteger(text.data(), text.size()));
	}

	/**
	* Generate an DamgardJurik key pair using the given parameters.
	* @param keyParams MUST be an instance of DJKeyGenParameterSpec.
	* @return KeyPair contains keys for this DamgardJurik encryption object.
	* @throws InvalidParameterSpecException if keyParams is not instance of DJKeyGenParameterSpec.
	*/
	pair<shared_ptr<PublicKey>, shared_ptr<PrivateKey>> generateKey(shared_ptr<AlgorithmParameterSpec> keyParams) override;

	/**
	* This function is not supported for this encryption scheme, since there is a need for parameters to generate a DamgardJurik key pair.
	* @throws UnsupportedOperationException
	*/
	pair<shared_ptr<PublicKey>, shared_ptr<PrivateKey>> generateKey() override{
		throw UnsupportedOperationException("Use generateKey function with DJKeyGenParameterSpec");
	}

	/**
	* Fix the length parameter for the encryption
	* @param s  Length parameter
	*/
	void setLengthParameter(int s) { this->consts = s; }

	/**
	* This function performs the encryption of he given plain text
	* @param plainText MUST be an instance of BigIntegerPlainText.
	* @return an object of type BigIntegerCiphertext holding the encryption of the plaintext.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If the given plaintext is not instance of BigIntegerPlainText.
	* 		2. If the BigInteger value in the given plaintext is not in ZN.
	*/
	shared_ptr<AsymmetricCiphertext> encrypt(shared_ptr<Plaintext> plaintext) override;

	/**
	* Encrypts the given plaintext using this asymmetric encryption scheme and using the given random value.<p>
	* There are cases when the random value is used after the encryption, for example, in sigma protocol.
	* In these cases the random value should be known to the user. We decided not to have function that return it to the user
	* since this can cause problems when more than one value is being encrypt.
	* Instead, we decided to have an additional encrypt value that gets the random value from the user.
	* @param plainText message to encrypt
	* @param r The random value to use in the encryption.
	* @param plainText MUST be an instance of BigIntegerPlainText.
	* @return an object of type BigIntegerCiphertext holding the encryption of the plaintext.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If the given plaintext is not instance of BigIntegerPlainText.
	* 		2. If the BigInteger value in the given plaintext is not in ZN.
	*/
	shared_ptr<AsymmetricCiphertext> encrypt(shared_ptr<Plaintext>, biginteger r) override;

	/**
	* Decrypts the given ciphertext using DamgardJurik encryption scheme.
	* @param cipher has to be an instance of BigIntegerCiphertext.
	* @throws KeyException if the Private Key has not been set for this object.
	* @throws IllegalArgumentException if cipher is not an instance of BigIntegerCiphertext.
	*/
	shared_ptr<Plaintext> decrypt(shared_ptr<AsymmetricCiphertext> cipher) override;

	/**
	* Generates a byte array from the given plaintext.
	* This function should be used when the user does not know the specific type of the Asymmetric encryption he has,
	* and therefore he is working on byte array.
	* @param plaintext to generates byte array from. MUST be an instance of BigIntegerPlainText.
	* @return the byte array generated from the given plaintext.
	* @throws IllegalArgumentException if the given plaintext is not an instance of BigIntegerPlainText.
	*/
	vector<byte> generateBytesFromPlaintext(shared_ptr<Plaintext> plaintext) override;

	/**
	* This function takes an encryption of some plaintext (let's call it originalPlaintext) and returns a cipher that "looks" different but
	* it is also an encryption of originalPlaintext.<p>
	* The given ciphertext have to has been generated with the same public key as this encryption's public key.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If cipher is not an instance of BigIntegerCiphertext.
	* 		2. If the BigInteger number in the given cipher is not in ZN'.
	*/
	shared_ptr<AsymmetricCiphertext> reRandomize(shared_ptr<AsymmetricCiphertext> cipher);

	/**
	* This function takes an encryption of some plaintext (let's call it originalPlaintext) and returns a cipher that "looks" different but
	* it is also an encryption of originalPlaintext. It uses the given BigInteger random value.<p>
	* The given ciphertext have to has been generated with the same public key as this encryption's public key.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If cipher is not an instance of BigIntegerCiphertext.
	* 		2. If the BigInteger number in the given cipher is not in ZN'.
	*/
	shared_ptr<AsymmetricCiphertext> reRandomize(shared_ptr<AsymmetricCiphertext> cipher, biginteger r);

	/**
	* Given two ciphers c1 = Enc(p1)  and c2 = Enc(p2) this function return c1 + c2 = Enc(p1 +p2).
	* Both ciphertext have to have been generated with the same public key as this encryption's public key.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If one or more of the given ciphertexts is not an instance of BigIntegerCiphertext.
	* 		2. If the sizes of ciphertexts do not match.
	* 		3. If one or more of the BigInteger numbers in the given ciphertexts is not in ZN'.
	*/
	shared_ptr<AsymmetricCiphertext> add(shared_ptr<AsymmetricCiphertext> cipher1, shared_ptr<AsymmetricCiphertext> cipher2) override;

	/**
	* Given two ciphers c1 = Enc(p1)  and c2 = Enc(p2) this function return c1 + c2 = Enc(p1 +p2).<p>
	* Both ciphertext have to have been generated with the same public key as this encryption's public key.<p>
	*
	* There are cases when the random value is used after the function, for example, in sigma protocol.
	* In these cases the random value should be known to the user. We decided not to have function that return it to the user
	* since this can cause problems when the add function is called more than one time.
	* Instead, we decided to have an additional add function that gets the random value from the user.
	*
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If one or more of the given ciphertexts is not an instance of BigIntegerCiphertext.
	* 		2. If the sizes of ciphertexts do not match.
	* 		3. If one or more of the BigInteger numbers in the given ciphertexts is not in ZN'.
	*/
	shared_ptr<AsymmetricCiphertext> add(shared_ptr<AsymmetricCiphertext> cipher1, shared_ptr<AsymmetricCiphertext> cipher2, biginteger r) override;

	/**
	* This function calculates the homomorphic multiplication by a constant of a ciphertext<p>
	* in the Damgard Jurik encryption scheme.
	* @param cipher the cipher to operate on.
	* @param constNumber the constant number by which to multiply the cipher.
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If the given cipher is not an instance of BigIntegerCiphertext.
	* 		2. If the BigInteger numbers in the given ciphertext is not in ZN'.
	* 		3. If the constant number is not in ZN.
	*/
	shared_ptr<AsymmetricCiphertext> multByConst(shared_ptr<AsymmetricCiphertext> cipher, biginteger constNumber) override;
	
	/**
	* This function calculates the homomorphic multiplication by a constant of a ciphertext
	* in the Damgard Jurik encryption scheme.<p>
	*
	* There are cases when the random value is used after the function, for example, in sigma protocol.
	* In these cases the random value should be known to the user. We decided not to have function that return it to the user
	* since this can cause problems when the add function is called more than one time.
	* Instead, we decided to have an additional add function that gets the random value from the user.
	*
	* @param cipher the cipher to operate on.
	* @param constNumber the constant number by which to multiply the cipher.
	* @param r The random value to use in the function.
	*
	* @throws IllegalStateException if no public key was set.
	* @throws IllegalArgumentException in the following cases:
	* 		1. If the given cipher is not an instance of BigIntegerCiphertext.
	* 		2. If the BigInteger numbers in the given ciphertext is not in ZN'.
	* 		3. If the constant number is not in ZN.
	*/
	shared_ptr<AsymmetricCiphertext> multByConst(shared_ptr<AsymmetricCiphertext> cipher, biginteger constNumber, biginteger r) override;

	shared_ptr<AsymmetricCiphertext> reconstructCiphertext(shared_ptr<AsymmetricCiphertextSendableData> data) override;

	shared_ptr<PublicKey> reconstructPublicKey(shared_ptr<KeySendableData> data) override;

	shared_ptr<PrivateKey> reconstructPrivateKey(shared_ptr<KeySendableData> data) override;

};

