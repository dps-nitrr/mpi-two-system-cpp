# MPI Data Exchange Between Two Systems (C++)

Distributed Systems assignment (Dr. Sonal Yadav): parallel C++ programs using the
Message Passing Interface (MPI) to exchange data between **System 1** and **System 2**.

**Mapping:** MPI rank 0 = System 1, MPI rank 1 = System 2. All programs need exactly 2 processes.

## 1. Repository layout

```
.
├── src/
│   ├── hello.cpp       # both systems report rank + hostname
│   ├── exchange.cpp    # send array -> square + sum -> return -> verify
│   └── pingpong.cpp    # text message, latency and bandwidth test
├── Makefile            # build + run targets
├── hostfile            # the two machines (edit before cluster run)
├── screenshots/        # execution screenshots (add yours here)
└── README.md
```

## 2. Prerequisites (on BOTH systems)

- Linux (Ubuntu 20.04+), both machines on the same network and able to ping each other
- Same OpenMPI version on both, same username on both, same project path on both

```bash
sudo apt update
sudo apt install -y openmpi-bin libopenmpi-dev build-essential openssh-server
mpicxx --version && mpirun --version
```

## 3. Part A: Quick test on a single machine

```bash
make              # builds into ./bin
make run-local    # runs all three programs with 2 processes
```

Or manually:

```bash
mpicxx -std=c++17 -O2 -o bin/exchange src/exchange.cpp
mpirun -np 2 bin/exchange
```
If OpenMPI complains about slots on a single machine, add `--oversubscribe`.

## 4. Part B: Run across two systems

**Step 1: IP addresses and names.** On each machine run `hostname -I`.
Add both to `/etc/hosts` on **both** machines (use your real IPs):

```
192.168.1.10  system1
192.168.1.11  system2
```

**Step 2: Passwordless SSH from System 1 to System 2.**

```bash
ssh-keygen -t rsa -b 4096          # press Enter for all prompts
ssh-copy-id <user>@system2
ssh system2 hostname               # must print without a password prompt
```

**Step 3: Get the code on both machines at the same path.**

```bash
# on System 1
git clone https://github.com/<your-username>/<repo-name>.git ~/mpi-two-system-cpp
# on System 2
git clone https://github.com/<your-username>/<repo-name>.git ~/mpi-two-system-cpp
```

**Step 4: Build on BOTH machines.**

```bash
cd ~/mpi-two-system-cpp && make
```

**Step 5: Check `hostfile`** lists `system1 slots=1` and `system2 slots=1`.

**Step 6: Run from System 1.**

```bash
cd ~/mpi-two-system-cpp
make run-cluster
```
or individually:

```bash
mpirun -np 2 --hostfile hostfile bin/hello
mpirun -np 2 --hostfile hostfile bin/exchange
mpirun -np 2 --hostfile hostfile bin/pingpong
```

## 5. Expected output

(Hostnames differ on a real two-machine run; line order may vary.)

`hello`
```
Hello from rank 0 of 2 (System 1) running on host 'system1'
Hello from rank 1 of 2 (System 2) running on host 'system2'
```

`exchange`
```
[System 1 | system1] Sending 10 integers to System 2: 1 2 3 4 5 6 7 8 9 10
[System 2 | system2] Received 10 integers from System 1
[System 2 | system2] Computed squares and sum (385); sending back
[System 1 | system1] Received squares from System 2: 1 4 9 16 25 36 49 64 81 100
[System 1 | system1] Received sum of squares: 385
[System 1 | system1] Verification: PASSED
```

`pingpong` prints the two text messages, then latency (us) and bandwidth (MiB/s). Values depend on your network.

## 6. Screenshots (put in `screenshots/`)

1. `01_versions.png`: `mpicxx --version` / `mpirun --version` on both machines
2. `02_ssh.png`: passwordless `ssh system2 hostname`
3. `03_build.png`: output of `make`
4. `04_hello.png`: `hello` showing two different hostnames
5. `05_exchange.png`: `exchange` ending in `Verification: PASSED`
6. `06_pingpong.png`: `pingpong` with latency and bandwidth

## 7. Testing

- `exchange` verifies returned values and prints PASSED/FAILED.
- Different hostnames in `hello` prove the processes run on separate machines.
- `mpirun -np 3 bin/exchange` should print `Error: run with exactly 2 processes`.

## 8. Troubleshooting

| Problem | Fix |
|---|---|
| `unable to launch ... ssh` | Passwordless SSH not set up (step 2) |
| `bin/hello: No such file` on System 2 | Not built / different path on System 2 |
| Hangs at startup | Firewall: allow traffic between the IPs, or `sudo ufw disable` (lab only) |
| `not enough slots available` | Add `--oversubscribe` or fix `slots=` in `hostfile` |
| Version mismatch errors | Install the same OpenMPI version on both |
| Wrong network interface | Add `--mca btl_tcp_if_include eth0` (use your interface name) |

## 9. How it works

`MPI_Init` starts the runtime; `MPI_Comm_rank` gives each process its id (0 or 1);
`MPI_Send`/`MPI_Recv` move typed buffers between ranks (over TCP between machines); tags keep
different messages apart; `MPI_Barrier` synchronises before timing; `MPI_Wtime` measures time;
`MPI_Finalize` shuts the runtime down. The C++ code uses the MPI C API (the MPI C++ bindings
were removed in MPI-3) together with `std::vector` and `std::string`.
