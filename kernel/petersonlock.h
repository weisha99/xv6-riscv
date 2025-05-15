#ifndef _PETERSONLOCK_H_
#define _PETERSONLOCK_H_

struct petersonlock {
  int active;      
  int flag[2];         
  int turn;             
  int lock_id;
};

#endif
