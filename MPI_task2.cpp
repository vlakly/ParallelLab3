#include <iostream>
#include <string>
#include "mpi.h"

using namespace std;

#define S 8
#define TAG_SEND_MATRIX_DATA 1
#define TAG_CALCULATIONS 2

MPI_Status status;

uint32_t** create_empty() {
	uint32_t** m = new uint32_t * [S];
	for (int i = 0; i < S; ++i) {
		m[i] = new uint32_t[S];
		for (int j = 0; j < S; ++j) {
			m[i][j] = 0;
		}
	}
	return m;
}
uint32_t** create_random() {
	uint32_t** m = new uint32_t * [S];
	for (int i = 0; i < S; ++i) {
		m[i] = new uint32_t[S];
		for (int j = 0; j < S; ++j) {
			m[i][j] = rand() % 20;
		}
	}
	return m;
}
void fill_random(uint32_t m[S][S]) {
	for (int i = 0; i < S; ++i) {
		for (int j = 0; j < S; ++j) {
			m[i][j] = rand() % 5;
		}
	}
}
void print_matrix(uint32_t** m) {
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < S; j++) {
			cout << m[i][j] << " ";
		}
		cout << "\n";
	}
	cout << "\n\n";
}
void multiply_matrices(uint32_t** res, uint32_t** mA, uint32_t** mB) {
	for (int i = 0; i < S; ++i) {
		for (int j = 0; j < S; ++j) {
			for (int k = 0; k < S; k++) {
				res[i][j] += mA[i][k] * mB[k][j];
			}
		}
	}
}
void multiply_matrices_MPI(uint32_t** res, uint32_t** mA, uint32_t** mB, int start, int end) {
	for (int i = 0; i < S; ++i) {
		for (int j = start; j < end; ++j) {
			for (int k = 0; k < S; k++) {
				res[i][j] += mA[i][k] * mB[k][j];
			}
		}
	}
}
bool check_equality(uint32_t** mA, uint32_t** mB) {
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < S; j++) {
			if (mA[i][j] != mB[i][j]) {
				return false;
			}
		}
	}
	return true;
}

int testCount = 10;
double avgTime = 0;

uint32_t matrix_a[S][S];
uint32_t matrix_b[S][S];
uint32_t matrix_simple[S][S];
uint32_t matrix_MPI[S][S];

uint32_t** MPI_res = create_empty();

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int threadID, threads, slaveCount, rows, offset;
	int masterID = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &threadID);
	MPI_Comm_size(MPI_COMM_WORLD, &threads);

	cout << "Current: " << threadID << ", overall: " << threads << "\n";

	if (threadID == 0) {
		fill_random(matrix_a);
		fill_random(matrix_b);

		slaveCount = threads - 1;
		rows = S / slaveCount;
		offset = 0;

		// dest - destination thread
		for (int dest = 1; dest <= slaveCount; dest++) {
			MPI_Send(&offset, 1, MPI_INT, dest, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD);
			MPI_Send(&matrix_a[offset][0], rows * S, MPI_UINT32_T, dest, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD);
			MPI_Send(&matrix_b, S * S, MPI_UINT32_T, dest, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD);

			offset = offset + rows;
			cout << "offset is " << offset << "\n";
		}

		// recieve calculation 

		for (int src = 1; src <= slaveCount; src++) {
			MPI_Recv(&offset, 1, MPI_INT, src, TAG_CALCULATIONS, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, src, TAG_CALCULATIONS, MPI_COMM_WORLD, &status);
			MPI_Recv(&MPI_res[offset][0], rows * S, MPI_UINT32_T, src, TAG_CALCULATIONS, MPI_COMM_WORLD, &status);
		}

		cout << "Final matrix\n";
		print_matrix(MPI_res);
		cout << "\n";

		cout << "why\n\n\n\n\n";
		

		//cout << "Result matix is \n";
		//print_matrix(MPI_res);
		//cout << "\n";

		//print_matrix(MPI_res);

		//int size = S / (threads - 1);
		//for (int i = 0; i < threads - 1; i++) {
		//	uint32_t ibeg = i * size;
		//	uint32_t iend = (i + 1) * size;
		//}
	}

	if (threadID > 0) {
		int source = 0;
		MPI_Recv(&offset, 1, MPI_INT, source, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, source, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD, &status);
		MPI_Recv(&matrix_a, rows * S, MPI_UINT32_T, source, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD, &status);
		MPI_Recv(&matrix_b, S * S, MPI_UINT32_T, source, TAG_SEND_MATRIX_DATA, MPI_COMM_WORLD, &status);
		cout << "OFFSET: " << offset << "\n";
		cout << "ROWS: " << rows << "\n";

		//cout << "Thread " << threadID << " recieved part of m A: \n";
		//print_matrix(matrix_a);
		//cout << "\n";
		//cout << "Thread " << threadID << " recieved m B: \n";
		//print_matrix(matrix_b);
		//cout << "\n";

		for (int k = 0; k < S; k++) {
			for (int i = 0; i < rows; i++) {
				MPI_res[i][k] = 0;
				for (int j = 0; j < S; j++) {
					MPI_res[i][k] += matrix_a[i][j] * matrix_b[j][k];
				}
			}
		}

		MPI_Send(&offset, 1, MPI_INT, masterID, TAG_CALCULATIONS, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, masterID, TAG_CALCULATIONS, MPI_COMM_WORLD);
		MPI_Send(&MPI_res, rows * S, MPI_UINT32_T, masterID, TAG_CALCULATIONS, MPI_COMM_WORLD);

		//cout << "Thread " << threadID << " result matix is \n";
		//print_matrix(MPI_res);
		//cout << "\n";

		//for (int k = 0; k < S; k++) {
		//	for (int i = 0; i < rows; i++) {
		//		MPI_res[i][k] = 0;
		//		for (int j = 0; j < S; j++) {
		//			//MPI_res[i][k] += mA[i][j] * mB[j][k];
		//		}
		//		//cout << MPI_res[i][k] << " ";
		//	}
		//	cout << "\n";
		//}
		//cout << "\n\n";
		//for (int i = 0; i < S; i++) {
		//	for (int j = 0; j < S; j++) {
		//		MPI_res[i][j] = j * i + j;
		//	}
		//}
	}
	MPI_Finalize();
}