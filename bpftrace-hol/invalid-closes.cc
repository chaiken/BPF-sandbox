#include <chrono>
#include <cstdio>
#include <thread>

/*
For exercise2-4.bt:

$ clang++ --std=c++17 bpftrace-hol/invalid-closes.cc -o
bpftrace-hol/invalid-closes

$ bpftrace-hol/invalid-closes
free(): double free detected in tcache 2
Aborted (core dumped)
 */

int main(int argc, char **argv) {
  std::FILE *filep = std::tmpfile();
  fclose(filep);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  fclose(filep);
}
