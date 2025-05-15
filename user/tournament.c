#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(2, "Usage: tournament number_of_processes\n");
    exit(1);
  }

  int n = atoi(argv[1]);
  
  // Create tournament tree
  int tournament_id = tournament_create(n);
  if (tournament_id < 0) {
    fprintf(2, "Failed to create tournament tree\n");
    exit(1);
  }

  // Acquire lock
  if (tournament_acquire() < 0) {
    fprintf(2, "Failed to acquire tournament lock\n");
    exit(1);
  }

  // Print message in critical section
  printf("Process %d (PID %d) in critical section\n", tournament_id, getpid());

  // Release lock
  if (tournament_release() < 0) {
    fprintf(2, "Failed to release tournament lock\n");
    exit(1);
  }

  exit(0);
}
