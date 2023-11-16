#include <iostream>
#include <string>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <stdio.h>
#include "mpi.h"

#define BOOST_UUID_COMPAT_PRE_1_71_MD5

#define TAG_START_DATA 1

MPI_Status status;

using boost::uuids::detail::md5;
using namespace std;

int password_length = 5;
char alphabet[] = { "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
int alphabet_length = sizeof(alphabet) / sizeof(alphabet[0]) - 1;
int symbols_in_password = (alphabet_length - 1) * password_length;

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
	string brutPassword = "00000";
	md5 hash;
	md5::digest_type brutDigest;
	for (int a = 0; a < alphabet_length; a++) {
		brutPassword[0] = alphabet[a];
		for (int b = 0; b < alphabet_length; b++) {
			brutPassword[1] = alphabet[b];
			for (int c = 0; c < alphabet_length; c++) {
				brutPassword[2] = alphabet[c];
				for (int d = 0; d < alphabet_length; d++) {
					brutPassword[3] = alphabet[d];
					for (int e = 0; e < alphabet_length; e++) {
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
string intToPassword(int num) {
	string password;

 	for (int i = password_length - 1; i >= 0; i--) {
		int current_num = num - (alphabet_length - 1) * i;
		int current_char;
		if (current_num > 0) {
			current_char = alphabet[current_num];
			num -= current_num;
		}
		else {
			current_char = alphabet[0];
		}
		password.push_back(current_char);
		//password.push_back('Z');
	}

	return password;
}

int main(int argc, char **argv) {

	//#ifdef BOOST_UUID_COMPAT_PRE_1_71_MD5
	//	cout << ("okay\n");
	//#endif
	//#ifndef BOOST_UUID_COMPAT_PRE_1_71_MD5
	//	cout << ("hell nah\n");
	//	return 0;
	//#endif

	srand(time(NULL));

	md5 hash;
	md5::digest_type passDigest;

	// random password
	// 
	//string password = generatePassword(password_length);
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//cout << "Generated password is " << password << "\n";
	//cout << "Digest of password is " << toString(passDigest) << "\n";

	// own password for test
	// 
	//string password = "0123C";
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//hash.process_bytes(password.data(), password.size());
	//hash.get_digest(passDigest);
	//cout << "Generated password is " << password << "\n";
	//cout << "Digest of test password is " << toString(passDigest) << "\n";

	//bruteForce(passDigest, password);	

	MPI_Init(&argc, &argv);

	int threadID, threads, slaveCount, rows, offset;

	MPI_Comm_rank(MPI_COMM_WORLD, &threadID);
	MPI_Comm_size(MPI_COMM_WORLD, &threads);

	if (threadID == 0) {
		string test;
		int a;
		a = 0;
		test = intToPassword(a);
		cout << "\n" << test << "\n";
		a = 1;
		test = intToPassword(a);
		cout << "\n" << test << "\n";
		a = 304;
		test = intToPassword(a);
		cout << "\n" << test << "\n";
		a = 305;
		test = intToPassword(a);
		cout << "\n" << test << "\n";
		cout << "\n" << symbols_in_password << "\n";

		//slaveCount = threads - 1;
		//rows = symbols_in_password / slaveCount;
		//offset = 0;

		cout << "===MPI BRUTEFORCE===\n";
		cout << "Thread count: " << threads << ", 1 - master, " << threads - 1 << " - slaves\n";
		cout << "ABC length (start = 0): " << alphabet_length << ", symbols in password (start = 0): " << symbols_in_password << "\n";

		//for (int dest = 1; dest <= slaveCount; dest++) {
		//	if (dest == slaveCount) {
		//		rows = symbols_in_password / slaveCount + symbols_in_password % slaveCount;
		//	}
		//	MPI_Send(&offset, 1, MPI_INT, dest, TAG_START_DATA, MPI_COMM_WORLD);
		//	MPI_Send(&rows, 1, MPI_INT, dest, TAG_START_DATA, MPI_COMM_WORLD);

		//	offset = offset + rows;
		//}
	}

	//if (threadID > 0) {
	//	int source = 0;
	//	MPI_Recv(&offset, 1, MPI_INT, source, TAG_START_DATA, MPI_COMM_WORLD, &status);
	//	MPI_Recv(&rows, 1, MPI_INT, source, TAG_START_DATA, MPI_COMM_WORLD, &status);

	//	cout << "Thr " << threadID << " offset: " << offset << " rows: " << rows << "\n";
	//}

	MPI_Finalize();

	return 0;
}