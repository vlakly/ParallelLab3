#include <iostream>
#include <string>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <stdio.h>
#include "mpi.h"

#define BOOST_UUID_COMPAT_PRE_1_71_MD5

using boost::uuids::detail::md5;
using namespace std;

string toString(const md5::digest_type& digest) {
	const auto charDigest = reinterpret_cast<const char*>(&digest);
	string result;
	boost::algorithm::hex(charDigest, charDigest + sizeof(md5::digest_type), back_inserter(result));
	return result;
}
string generatePassword(int length) {
	string password;
	for (int i = 0; i < length; i++) {
		int ascii = 0;
		switch (rand() % 3) {
		case(0):
			ascii = 48 + (rand() % 10); // 0-9
			break;
		case(1):
			ascii = 65 + (rand() % 26); // A-Z
			break;
		case(2):
			ascii = 97 + (rand() % 26); // a-z
			break;
		}
		char ch = char(ascii);
		password += ch;
	}
	return password;
}
string bruteForce(const md5::digest_type& passDigest, string password) {
	char alphabet[] = { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	int alphabetLength = sizeof(alphabet) / sizeof(alphabet[0]) - 1;
	string brutPassword = "00000";
	md5 hash;
	md5::digest_type brutDigest;
	for (int a = 0; a < alphabetLength; a++) {
		brutPassword[0] = alphabet[a];
		for (int b = 0; b < alphabetLength; b++) {
			brutPassword[1] = alphabet[b];
			for (int c = 0; c < alphabetLength; c++) {
				brutPassword[2] = alphabet[c];
				for (int d = 0; d < alphabetLength; d++) {
					brutPassword[3] = alphabet[d];
					for (int e = 0; e < alphabetLength; e++) {
						brutPassword[4] = alphabet[e];
						hash.process_bytes(brutPassword.data(), brutPassword.size());
						hash.get_digest(brutDigest);
						hash.process_bytes(brutPassword.data(), brutPassword.size());
						hash.get_digest(brutDigest);
						if (toString(brutDigest) == toString(passDigest)) {
							cout << "Digest of given password is " << toString(passDigest) << "\n";
							cout << "Digest of brut password is " << toString(brutDigest) << "\n";
							cout << "Password: " << brutPassword << "\n";
							return brutPassword;
						}
						cout << "brut: " << brutPassword << ", digest: " << toString(brutDigest) << "\n";
					}
				}
			}
		}
	}
	return "-1";
}

int main(int *argc, char **argv) {

	//#ifdef BOOST_UUID_COMPAT_PRE_1_71_MD5
	//	cout << ("okay\n");
	//#endif
	//#ifndef BOOST_UUID_COMPAT_PRE_1_71_MD5
	//	cout << ("hell nah\n");
	//	return 0;
	//#endif

	srand(time(NULL));


	int passwordLength = 5;

	md5 hash;
	md5::digest_type passDigest;

	// random password
	// 
	//string password = generatePassword(passwordLength);
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//cout << "Generated password is " << password << "\n";
	//cout << "Digest of password is " << toString(passDigest) << "\n";

	// own password for test
	// 
	string password = "0123C";
	hash.process_bytes(password.data(), password.size());
	hash.get_digest(passDigest);
	hash.process_bytes(password.data(), password.size());
	hash.get_digest(passDigest);
	cout << "Generated password is " << password << "\n";
	cout << "Digest of test password is " << toString(passDigest) << "\n";

	bruteForce(passDigest, password);	

	//cout << "Hello MPI\n";

	//int numtasks, rank;

	//MPI_Init(argc, &argv);

	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	//printf("Hello MPI from process = %d, total number of processes: %d\n", rank, numtasks);

	//MPI_Finalize();
}