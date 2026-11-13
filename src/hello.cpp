// hello.cpp - Verifies that MPI processes start on both systems.
// Each process reports its rank, the total process count and its hostname.
#include <iostream>
#include <sstream>
#include <string>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank = 0, size = 0, name_len = 0;
    char host[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Get_processor_name(host, &name_len);

    std::ostringstream out;
    out << "Hello from rank " << rank << " of " << size
        << " (System " << rank + 1 << ") running on host '" << host << "'\n";
    std::cout << out.str() << std::flush;

    MPI_Finalize();
    return 0;
}
