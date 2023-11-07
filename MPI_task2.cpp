#include <iostream>
#include <string>
#include "mpi.h"

using namespace std;

uint32_t** create_empty(int size) {
	uint32_t** m = new uint32_t * [size];
	for (int i = 0; i < size; ++i) {
		m[i] = new uint32_t[size];
		for (int j = 0; j < size; ++j) {
			m[i][j] = 0;
		}
	}
	return m;
}
uint32_t** create_random(int size) {
	uint32_t** m = new uint32_t * [size];
	for (int i = 0; i < size; ++i) {
		m[i] = new uint32_t[size];
		for (int j = 0; j < size; ++j) {
			m[i][j] = rand() % 5;
		}
	}
	return m;
}
void print_matrix(uint32_t** m, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			cout << m[i][j] << " ";
		}
		cout << "\n";
	}
	cout << "\n\n";
}
void multiply_matrices(uint32_t** res, uint32_t** mA, uint32_t** mB, int size) {
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			for (int k = 0; k < size; k++) {
				res[i][j] += mA[i][k] * mB[k][j];
			}
		}
	}
}
bool check_equality(uint32_t** mA, uint32_t** mB, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (mA[i][j] != mB[i][j]) {
				return false;
			}
		}
	}
	return true;
}

int main() {
	cout << "Hello MPI\n";
}