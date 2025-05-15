#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "petersonlock.h"

#define MAX_PETERSON_LOCKS 15

struct petersonlock peterson_locks[MAX_PETERSON_LOCKS];

void
initpetersonlocks(void)
{
  for(int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    peterson_locks[i].active = 0;
    peterson_locks[i].flag[0] = 0;
    peterson_locks[i].flag[1] = 0;
    peterson_locks[i].turn = 0;
  }
}

int
sys_peterson_create(void) {
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    if (__sync_lock_test_and_set(&peterson_locks[i].active, 1) == 0) {
      peterson_locks[i].flag[0] = 0;
      peterson_locks[i].flag[1] = 0;
      peterson_locks[i].turn = 0;
      return i;
    }
  }
  return -1;
}

int
sys_peterson_acquire(int lock_id, int role) {
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1)
    return -1;

  struct petersonlock *l = &peterson_locks[lock_id];
  if (!l->active)
    return -1;

  int other = 1 - role;
  __sync_lock_test_and_set(&l->flag[role], 1);
  __sync_synchronize();
  l->turn = other;
  __sync_synchronize();

  while (l->flag[other] && l->turn == other) {
    yield();
    __sync_synchronize();
  }

  return 0;
}

int
peterson_release(int lock_id, int role) {
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1)
    return -1;

  struct petersonlock *l = &peterson_locks[lock_id];
  if (!l->active)
    return -1;

  __sync_synchronize();
  __sync_lock_release(&l->flag[role]);
  return 0;
}

int
peterson_destroy(int lock_id) {
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS)
    return -1;

  struct petersonlock *l = &peterson_locks[lock_id];
  if (!l->active)
    return -1;

  l->active = 0;
  return 0;
}
