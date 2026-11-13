// exchange.cpp - Two-way data exchange between System 1 (rank 0) and System 2 (rank 1).
//
// System 1: builds an integer array and sends it to System 2.
// System 2: receives it, squares every element, computes the sum,
//           and sends the squared array and the sum back.
// System 1: receives the results, prints them and verifies correctness.
//
// Run with exactly 2 processes.
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mpi.h>

namespace {
constexpr int N          = 10;
constexpr int TAG_DATA   = 1;
constexpr int TAG_SQUARE = 2;
constexpr int TAG_SUM    = 3;

std::string join(const std::vector<int> &v) {
    std::ostringstream s;
    for (int x : v) s << ' ' << x;
    return s.str();
}
void say(const std::string &line) { std::cout << line << std::endl; }
}  // namespace

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank = 0, size = 0, name_len = 0;
    char host_buf[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(host_buf, &name_len);
    const std::string host(host_buf);

    if (size != 2) {
        if (rank == 0)
            std::cerr << "Error: run with exactly 2 processes (got " << size << ").\n";
        MPI_Finalize();
        return 1;
    }

    MPI_Status st;

    if (rank == 0) {  // ------------------------- System 1 -------------------------
        std::vector<int> data(N), squares(N);
        for (int i = 0; i < N; ++i) data[i] = i + 1;
        int sum = 0;

        say("[System 1 | " + host + "] Sending " + std::to_string(N) +
            " integers to System 2:" + join(data));
        MPI_Send(data.data(), N, MPI_INT, 1, TAG_DATA, MPI_COMM_WORLD);

        MPI_Recv(squares.data(), N, MPI_INT, 1, TAG_SQUARE, MPI_COMM_WORLD, &st);
        MPI_Recv(&sum, 1, MPI_INT, 1, TAG_SUM, MPI_COMM_WORLD, &st);

        say("[System 1 | " + host + "] Received squares from System 2:" + join(squares));
        say("[System 1 | " + host + "] Received sum of squares: " + std::to_string(sum));

        bool ok = true;
        int expected = 0;
        for (int i = 0; i < N; ++i) {
            expected += data[i] * data[i];
            if (squares[i] != data[i] * data[i]) ok = false;
        }
        if (expected != sum) ok = false;
        say("[System 1 | " + host + "] Verification: " + (ok ? "PASSED" : "FAILED"));

    } else {  // ------------------------------ System 2 -----------------------------
        std::vector<int> data(N), squares(N);
        int sum = 0;

        MPI_Recv(data.data(), N, MPI_INT, 0, TAG_DATA, MPI_COMM_WORLD, &st);
        say("[System 2 | " + host + "] Received " + std::to_string(N) + " integers from System 1");

        for (int i = 0; i < N; ++i) {
            squares[i] = data[i] * data[i];
            sum += squares[i];
        }
        say("[System 2 | " + host + "] Computed squares and sum (" + std::to_string(sum) +
            "); sending back");

        MPI_Send(squares.data(), N, MPI_INT, 0, TAG_SQUARE, MPI_COMM_WORLD);
        MPI_Send(&sum, 1, MPI_INT, 0, TAG_SUM, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
