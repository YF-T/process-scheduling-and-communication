// shared memory

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "shm.h"
#include "memlayout.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

struct sharedmemory shm[NSHM];

int
shmalloc()
{
  for(int i = 0; i < NSHM; i++){
    acquire(&shm[i].lock);
    if(shm[i].status == 0){
      shm[i].status = 1;
      shm[i].addr = kalloc();
      if(shm[i].addr == 0){
        shm[i].status = 0;
        release(&shm[i].lock);
        return -1;
      }
      release(&shm[i].lock);
      return i;
    }
    release(&shm[i].lock);
  }
  return -1;
}

void
shmfree(int shmid)
{
  acquire(&shm[shmid].lock);
  kfree(shm[shmid].addr);
  shm[shmid].status = 0;
  release(&shm[shmid].lock);
}

int
shmread(int shmid, void* buff, int n)
{
  acquire(&shm[shmid].lock);
  if(n > shm[shmid].length){
    n = shm[shmid].length;
  }
  struct proc* p = myproc();
  copyout(p->pagetable, (uint64)buff, (char*)shm[shmid].addr, n);
  release(&shm[shmid].lock);
  return n;
}

int
shmwrite(int shmid, void* buff, int n)
{
  acquire(&shm[shmid].lock);
  if(n > PGSIZE){
    release(&shm[shmid].lock);
    return -1;
  }
  struct proc* p = myproc();
  copyin(p->pagetable, (char*)shm[shmid].addr, (uint64)buff, n);
  shm[shmid].length = n;
  release(&shm[shmid].lock);
  return n;
}