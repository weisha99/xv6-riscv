#include "kernel/types.h"
#include "user/user.h"

static struct {
  int *locks;           // Array of Peterson locks in BFS order
  int num_processes;    // Number of processes (must be power of 2)
  int num_levels;       // Number of levels in tree (log2(num_processes))
  int process_index;    // Index of this process (0 to num_processes-1)
  int initialized;      // Whether tournament tree is initialized
} tournament;

static int
is_power_of_2(int n) {
  return n > 0 && (n & (n - 1)) == 0;
}

static int
log2(int n) {
  int result = 0;
  while (n > 1) {
    n >>= 1;
    result++;
  }
  return result;
}

static int
get_role(int level) {
  return (tournament.process_index & (1 << (tournament.num_levels - level - 1))) >> 
         (tournament.num_levels - level - 1);
}

static int
get_lock_level_index(int level) {
  return tournament.process_index >> (tournament.num_levels - level);
}

static int
get_lock_array_index(int level) {
  return get_lock_level_index(level) + (1 << level) - 1;
}

int
tournament_create(int processes) {
  // Validate input
  if (!is_power_of_2(processes) || processes > 16 || processes < 2)
    return -1;

  // Calculate number of levels
  int num_levels = log2(processes);
  
  // Calculate total number of locks needed
  int num_locks = (1 << num_levels) - 1;  // 2^L - 1 internal nodes
  
  // Allocate lock array
  int *locks = malloc(num_locks * sizeof(int));
  if (!locks)
    return -1;

  // Create all Peterson locks
  for (int i = 0; i < num_locks; i++) {
    int lock_id = peterson_create();
    if (lock_id < 0) {
      // Cleanup could be implemented here
      return -1;
    }
    locks[i] = lock_id;
  }

  // Fork processes
  int pid;
  for (int i = 1; i < processes; i++) {
    pid = fork();
    if (pid < 0) {
      return -1;
    }
    if (pid == 0) {  // Child
      tournament.process_index = i;
      tournament.locks = locks;
      tournament.num_processes = processes;
      tournament.num_levels = num_levels;
      tournament.initialized = 1;
      return i;
    }
  }

  // Parent becomes process 0
  tournament.process_index = 0;
  tournament.locks = locks;
  tournament.num_processes = processes;
  tournament.num_levels = num_levels;
  tournament.initialized = 1;
  return 0;
}

int
tournament_acquire(void) {
  if (!tournament.initialized)
    return -1;

  // Acquire locks from bottom to top
  for (int level = tournament.num_levels - 1; level >= 0; level--) {
    int lock_array_index = get_lock_array_index(level);
    int role = get_role(level);
    
    if (peterson_acquire(tournament.locks[lock_array_index], role) < 0)
      return -1;
  }

  return 0;
}

int
tournament_release(void) {
  if (!tournament.initialized)
    return -1;

  // Release locks from top to bottom
  for (int level = 0; level < tournament.num_levels; level++) {
    int lock_array_index = get_lock_array_index(level);
    int role = get_role(level);
    
    if (peterson_release(tournament.locks[lock_array_index], role) < 0)
      return -1;
  }

  return 0;
}
