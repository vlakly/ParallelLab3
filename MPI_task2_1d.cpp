#include <iostream>
#include <string>
#include "mpi.h"

using namespace std;

#define S 2048
#define TAG_START_DATA 1
#define TAG_MULTIPLICATION_DATA 2

MPI_Status status;

void MAT_fill_empty(uint32_t *m) {
	for (int i = 0; i < S * S; i++) {
		m[i] = 0;
	}
}
void MAT_fill_random(uint32_t *m) {
	for (int i = 0; i < S * S; i++) {
		m[i] = rand() % 4;
	}
}
void MAT_print(uint32_t *m) {
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < S; j++) {
			cout << m[j + i * S] << " ";
		}
		cout << "\n";
	}
}
void MAT_scalar_multiply(uint32_t *res, uint32_t* mA, uint32_t* mB) {
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < S; j++) {
			for (int k = 0; k < S; k++) {
				res[j + i * S] += mA[k + i * S] * mB[j + k * S];
			}
		}
	}
}
void MAT_multiply_MPI(uint32_t *res, uint32_t *mA, uint32_t *mB, int start, int end) {
	for (int i = start; i < end; ++i) {
		for (int j = 0; j < S; ++j) {
			for (int k = 0; k < S; k++) {
				res[j + i * S] += mA[k + i * S] * mB[j + k * S];
			}
		}
	}
}
bool MAT_check_equality(uint32_t *mA, uint32_t *mB) {
	for (int i = 0; i < S; i++) {
		for (int j = 0; j < S; j++) {
			if (mA[j + i * S] != mB[j + i * S]) {
				return false;
			}
		}
	}
	return true;
}

int testCount = 1;
double avgTime = 0;

uint32_t mat_a[S * S];
uint32_t mat_b[S * S];
uint32_t mat_SCALAR[S * S];
uint32_t mat_MPI[S * S];

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int threadID, threads, slaveCount, rows, offset;

	MPI_Comm_rank(MPI_COMM_WORLD, &threadID);
	MPI_Comm_size(MPI_COMM_WORLD, &threads);

	if (threadID == 0) {
		cout << "===MPI MULTIPLICATION===\n";
		cout << "Thread count: " << threads << ", 1 - master, " << threads - 1 << " - slaves\n";
	}

	for (int i = 0; i < testCount; i++) {
		int masterID = 0;

		double t_start, t_end;

		//cout << "Current: " << threadID << ", overall: " << threads << "\n";
		t_start = MPI_Wtime();
		if (threadID == 0) {
			cout << "Test case #" << i << "\n";
			slaveCount = threads - 1;
			rows = S / slaveCount;
			offset = 0;

			MAT_fill_random(mat_a);
			MAT_fill_random(mat_b);
			MAT_fill_empty(mat_MPI);

			MAT_fill_empty(mat_SCALAR);
			MAT_scalar_multiply(mat_SCALAR, mat_a, mat_b);

			//cout << "\nSCALAR MATRIX RESULT\n";
			//MAT_print(mat_SCALAR);
			//cout << "\n";

			// dest - destination thread
			for (int dest = 1; dest <= slaveCount; dest++) {
				if (dest == slaveCount) {
					rows = S / slaveCount + S % slaveCount;
				}
				MPI_Send(&offset, 1, MPI_INT, dest, TAG_START_DATA, MPI_COMM_WORLD);
				MPI_Send(&rows, 1, MPI_INT, dest, TAG_START_DATA, MPI_COMM_WORLD);
				MPI_Send(&mat_a[offset * S], rows * S, MPI_UINT32_T, dest, TAG_START_DATA, MPI_COMM_WORLD);
				MPI_Send(&mat_b, S * S, MPI_UINT32_T, dest, TAG_START_DATA, MPI_COMM_WORLD);

				offset = offset + rows;
			}

			// recieve calculation 
			for (int src = 1; src <= slaveCount; src++) {
				MPI_Recv(&offset, 1, MPI_INT, src, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD, &status);
				MPI_Recv(&rows, 1, MPI_INT, src, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD, &status);
				MPI_Recv(&mat_MPI[offset * S], rows * S, MPI_UINT32_T, src, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD, &status);
			}

			//cout << "\nMPI MATRIX RESULT\n";
			//MAT_print(mat_MPI);
			//cout << "\n";

			if (MAT_check_equality(mat_SCALAR, mat_MPI)) {
				cout << "Matrices are equal\n";
			}
			else {
				cout << "Matrices aren't equal\n";
			}
		}

		if (threadID > 0) {
			int source = 0;
			MPI_Recv(&offset, 1, MPI_INT, source, TAG_START_DATA, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, TAG_START_DATA, MPI_COMM_WORLD, &status);
			MPI_Recv(&mat_a[offset * S], rows * S, MPI_UINT32_T, source, TAG_START_DATA, MPI_COMM_WORLD, &status);
			MPI_Recv(&mat_b, S * S, MPI_UINT32_T, source, TAG_START_DATA, MPI_COMM_WORLD, &status);

			MAT_multiply_MPI(mat_MPI, mat_a, mat_b, offset, offset + rows);

			//cout << "Thread " << threadID << " offset: " << offset << ", rows: " << rows << "\n";
			//cout << "Thread " << threadID << " result\n";
			//MAT_print(mat_MPI);
			//cout << "\n";

			//cout << "Thread " << threadID << " mat A\n";
			//MAT_print(mat_a);
			//cout << "\n";
			//cout << "Thread " << threadID << " mat B\n";
			//MAT_print(mat_b);
			//cout << "\n";

			MPI_Send(&offset, 1, MPI_INT, masterID, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, masterID, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD);
			MPI_Send(&mat_MPI[offset * S], rows * S, MPI_UINT32_T, masterID, TAG_MULTIPLICATION_DATA, MPI_COMM_WORLD);
		}
		t_end = MPI_Wtime();

		if (threadID == 0) {
			cout << "MPI time: " << t_end - t_start << " s\n";
		}

		if (threadID == 0) {
			t_start = MPI_Wtime();
			MAT_scalar_multiply(mat_SCALAR, mat_a, mat_b);
			t_end = MPI_Wtime();
			cout << "Scalar time: " << t_end - t_start << " s\n";
		}
	}
	MPI_Finalize();

	return 0;
}