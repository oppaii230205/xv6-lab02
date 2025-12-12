#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 start_addr;
  int num_pages;
  uint64 user_buffer_addr;

  // Get system call arguments
  argaddr(0, &start_addr);
  argint(1, &num_pages);
  argaddr(2, &user_buffer_addr);
  
  // Validate num_pages (must fit in a 64-bit bitmask)
  if (num_pages < 0 || num_pages > 64) {
    return -1;
  }
    
  uint64 access_bitmask = 0;
  struct proc *current_proc = myproc();

  // Check each page for access bit
  for (int i = 0; i < num_pages; i++) {
    uint64 page_addr = start_addr + i * PGSIZE;
    pte_t *page_entry = walk(current_proc->pagetable, page_addr, 0);

    // Check if PTE exists and is valid
    if (page_entry == 0) {
      continue; // Skip non-existent pages
    }

    // Check if page is valid and has been accessed
    if ((*page_entry & PTE_V) && (*page_entry & PTE_A)) {
      access_bitmask |= (1ULL << i);
      *page_entry &= ~PTE_A; // Clear the access bit
    }
  }

  // Copy result back to user space
  if (copyout(current_proc->pagetable, user_buffer_addr, 
              (char *)&access_bitmask, sizeof(access_bitmask)) < 0) {
    return -1;
  }

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

