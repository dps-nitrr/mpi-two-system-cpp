// pingpong.cpp - Message exchange and network performance test between two systems.
//
// Part 1: System 1 sends a text message, System 2 replies.
// Part 2: Round-trip latency (small message, many repetitions).
// Part 3: Bandwidth (large message, several repetitions).
//
// Run with exactly 2 processes.
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>

namespace {
constexpr int    LAT_REPS = 1000;
constexpr int    BW_REPS  = 20;
constexpr int    BW_BYTES = 1024 * 1024;  // 1 MiB
constexpr size_t MSG_MAX  = 128;

void say(const std::string &line) { std::cout << line << std::endl; }

void send_text(const std::string &s, int dest) {
    MPI_Send(s.c_str(), static_cast<int>(s.size()) + 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
}
std::string recv_text(int src) {
    std::vector<char> buf(MSG_MAX);
    MPI_Status st;
    MPI_Recv(buf.data(), static_cast<int>(buf.size()), MPI_CHAR, src, 0, MPI_COMM_WORLD, &st);
    return std::string(buf.data());
}
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

    // ---- Part 1: text message ----
    if (rank == 0) {
        send_text("Hello System 2, this is System 1!", 1);
        say("[System 1 | " + host + "] Reply received: \"" + recv_text(1) + "\"");
    } else {
        say("[System 2 | " + host + "] Message received: \"" + recv_text(0) + "\"");
        send_text("Hello System 1, System 2 received your message!", 0);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // ---- Part 2: latency ----
    int token = 0;
    double t0 = MPI_Wtime();
    for (int i = 0; i < LAT_REPS; ++i) {
        if (rank == 0) {
            MPI_Send(&token, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(&token, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &st);
        } else {
            MPI_Recv(&token, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &st);
            MPI_Send(&token, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    }
    double t1 = MPI_Wtime();
    if (rank == 0) {
        double rtt_us = (t1 - t0) / LAT_REPS * 1e6;
        char line[160];
        std::snprintf(line, sizeof line,
                      "[System 1] Latency: %d round trips, avg round-trip = %.2f us (one-way ~ %.2f us)",
                      LAT_REPS, rtt_us, rtt_us / 2.0);
        say(line);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // ---- Part 3: bandwidth ----
    std::vector<char> buf(BW_BYTES, 'A');
    t0 = MPI_Wtime();
    for (int i = 0; i < BW_REPS; ++i) {
        if (rank == 0) {
            MPI_Send(buf.data(), BW_BYTES, MPI_CHAR, 1, 2, MPI_COMM_WORLD);
            MPI_Recv(buf.data(), BW_BYTES, MPI_CHAR, 1, 2, MPI_COMM_WORLD, &st);
        } else {
            MPI_Recv(buf.data(), BW_BYTES, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &st);
            MPI_Send(buf.data(), BW_BYTES, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
        }
    }
    t1 = MPI_Wtime();
    if (rank == 0) {
        double total_mib = 2.0 * BW_REPS * BW_BYTES / (1024.0 * 1024.0);
        char line[160];
        std::snprintf(line, sizeof line,
                      "[System 1] Bandwidth: %d x 1 MiB round trips, %.2f MiB/s",
                      BW_REPS, total_mib / (t1 - t0));
        say(line);
    }

    MPI_Finalize();
    return 0;
}
