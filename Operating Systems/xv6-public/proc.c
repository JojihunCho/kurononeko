#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "traps.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  int boost_time;
} ptable;

static struct proc *initproc;
int mlfq_cpu;
int mlfq_step = 0;
int min_step;

/* int procl0 = 0;
int procl1 = 0;
int procl2 = 0; */

int nexttid = 1;
int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  mlfq_cpu = 100;
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();

  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;
  int i;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->level = 0;
  p->runtime = 0;
  p->stride = 0;
  p->extra_slice = 0;
  p->lwpon = 0;
  p->havet = 0;
  p->numthread = 0;
  p->threadid = 0;
  p->ret_val = 0;
  p->s_base = 0;

  for(i = 0; i < 16; i++){
      p->usepage[i] = 0;
      p->page_base[i] = 0;
  }
//  procl0 = procl0 + 1;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}


//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd, hasT;
  

//  procl0 = procl0 - 1;
  if(curproc == initproc)
    panic("init exiting");
  
  if(curproc->threadid){
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
          if(p == curproc->parent){
              acquire(&ptable.lock);
              p->killed = 1; 
              if(p->state == SLEEPING)
                  p->state = RUNNABLE;
              release(&ptable.lock);
          }
      }
  }



  while(curproc->numthread){
      hasT = 1;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      acquire(&ptable.lock);
      
      if(p->threadid == 0){
          release(&ptable.lock);
          continue;
      }
      if(p->parent == curproc && p->state == ZOMBIE){
        hasT = 0;
        curproc->numthread--;
        kfree(p->kstack);
        p->kstack = 0;
        p->parent->usepage[p->havet] = 0;
        deallocuvm(p->pgdir, p->s_base + 2 * PGSIZE, p->s_base);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->lwpon = 0;
        p->havet = 0;
        p->numthread = 0;
        p->threadid = 0;
        p->ret_val = 0;
        p->s_base = 0;

        p->state = UNUSED;
        release(&ptable.lock);
      }else if(p->parent == curproc && p->state == EMBRYO){
          hasT = 0;
          curproc->numthread--;
          p->pid = 0;
          release(&ptable.lock);
      }
      else if(p->parent == curproc && p->state != ZOMBIE){
        hasT = 0;
        p->killed = 1;
        if(p->state == SLEEPING)
            p->state = RUNNABLE;
        release(&ptable.lock);

      }
    }
  if(hasT)
      break;
  }




  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();


  acquire(&ptable.lock);
  if(curproc->stride)
      return_cpu();
  


  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep

  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct proc *q;

  struct cpu *c = mycpu();
  c->proc = 0;
  int i;
  int beforetime;
      for(;;){
    // Enable interrupts on this processor.
    sti();
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    
    for(i = 0; i < 1; i++){
        beforetime = ptable.boost_time;
        
        p = fminstep();
        if(p){
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            switchkvm();

            c->proc = 0;
            continue;
        }
        

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE || p->stride != 0)
        continue;
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      if(p->level == 0){
            q = fminstep();
            if(q != 0)
                p = q;

            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            switchkvm();


            c->proc = 0;
      }
    }
    if(beforetime != ptable.boost_time)
        continue;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE || p->stride != 0)
            continue;
        
        if(p->level == 1){
            q = fminstep();
            if(q != 0)
                p = q;
            
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;


            swtch(&(c->scheduler), p->context);
            switchkvm();

            c->proc = 0;
          }
    }
    if(beforetime != ptable.boost_time)
        continue;


    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE || p->stride != 0)
            continue;

        if(p->level == 2){
            q = fminstep();
            if(q != 0)
                p = q;

            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            swtch(&(c->scheduler), p->context);
            switchkvm();

            c->proc = 0;
              }
          }
      } 

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      
    release(&ptable.lock);
    }
}

struct proc*    
fminstep()
{
    struct proc *run = 0;
    struct proc *p;
    int minnum = mlfq_step;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state != RUNNABLE)
            continue;

        if(p->stride == 0)
            continue;

        if(p->step < minnum){
            minnum = p->step;
            run = p;
        }
    }

    min_step = minnum;

    if(run != 0){
        run->step = run->step + 1000/run->shared_cpu;
    }
    else{
        mlfq_step = mlfq_step + 1000/mlfq_cpu;
    }

    return run;
}




void
boost(void)
{
    struct proc *p;

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->stride == 0||p->state == RUNNABLE){
          /*  if(p->level == 1){
                procl0 = procl0 + 1;
                procl1 = procl1 - 1;
            }
            if(p->level == 1){
                procl0 = procl0 + 1;
                procl2 = procl2 - 1;
            } */
            p->level = 0;
        
            p->runtime = 0;
        }
    }

    ptable.boost_time = 0;
}


// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();
 

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);

  if(myproc()->level == 1)
      myproc()->extra_slice = 1;
  if(myproc()->level == 2)
      myproc()->extra_slice = 3;


  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  int temp = myproc()->runtime;
  int notusetime = myproc()->extra_slice;
  acquire(&ptable.lock);  //DOC: yieldlock
  

  if(myproc()->level == 2){
      mlfq_step = mlfq_step + 1000/mlfq_cpu * 3;
      ptable.boost_time = ptable.boost_time + 4 - notusetime;
  }

  if(myproc()->level == 1){
      myproc()->runtime = temp + 2 - notusetime;
      ptable.boost_time = ptable.boost_time + 2 - notusetime;
      if(myproc()->runtime <= 10){
          myproc()->level = 2;
          myproc()->runtime = 0;
       //   procl1 = procl1 - 1;
       //   procl2 = procl2 + 1;
      }
      mlfq_step = mlfq_step + 1000/mlfq_cpu;
  }
  if(myproc()->level == 0 && myproc()->stride == 0){
      myproc()->runtime = temp + 1;
      ptable.boost_time = ptable.boost_time + 1;
      if(myproc()->runtime <= 5){
          myproc()->level = 1;
          myproc()->runtime = 0;
       //   procl1 = procl1 + 1;
       //   procl0 = procl0 - 1;
      }
  }
  
  if(ptable.boost_time >= 100)
      boost();


  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;
  int i = 0;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      i = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
    }
  }

  release(&ptable.lock);
  if(i)
    return 0;
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}




int
getppid(void)
{
    return myproc()->parent->pid;
}


int
getlev(void)
{
    return myproc()->level;
}


int
set_cpu_share(int pcpu)
{
    if(mlfq_cpu - pcpu < 20)
        return -1;
    mlfq_cpu = mlfq_cpu - pcpu;
    myproc()->stride = 1;
    myproc()->shared_cpu = pcpu;
    myproc()->step = min_step;
    return 0;
}

void return_cpu()
{
    mlfq_cpu = mlfq_cpu + myproc()->shared_cpu;
    myproc()->stride = 0;
    myproc()->shared_cpu = 0;
    myproc()->step = 0;
}

//for thread

int 
thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg)
{
    int i, ustack[2];
    struct proc *np;
    struct proc *curproc = myproc();
    struct proc *pprent;
    
    if((np = allocproc()) == 0){
        return -1;
    }
    
    acquire(&ptable.lock);

    if(curproc->lwpon == 0){
        pprent = curproc;
        np->pid = curproc->pid;
        curproc->numthread++;
        np->parent = pprent;
    }else{
        pprent = curproc->parent;
        np->pid = curproc->pid;
        pprent->numthread++;
        np->parent = pprent;
    }
    np->state = EMBRYO;
    release(&ptable.lock);

    if(pprent->numthread == 1)
        pprent->s_base = pprent->tf->esp;

    for(i = 0; i < NOFILE; i++)
        if(curproc->ofile[i])
            np->ofile[i] = filedup(curproc->ofile[i]);
    np->cwd = idup(curproc->cwd);


    safestrcpy(np->name, curproc->name, sizeof(curproc->name));
    np->sz = curproc->sz;
    np->pgdir = curproc->pgdir;
    np->pid = curproc->pid;
    *np->tf = *curproc->tf;
    np->lwpon = 1;

    np->threadid = nexttid++;
    *thread = np->threadid;
    
    //make stack frame
    for(i = 0; i < 16; i++){
        if(pprent->usepage[i])
            continue;
        if(pprent->page_base[i] == 0){   //set new s_base
            pprent->page_base[i] = pprent->s_base;
        }

        if(pprent->usepage[i] == 0)
            break;
    }

    np->sz = allocuvm(np->pgdir, pprent->page_base[i], pprent->page_base[i] + 2 * PGSIZE);
    np->s_base = pprent->page_base[i];
    pprent->s_base = np->sz;

    pprent->usepage[i] = 1;

    np->havet = i;      //save where it used

    np->tf->esp = np->sz;

    ustack[0] = 0xffffffff;  // fake return PC
    ustack[1] = (uint) arg;


    np->tf->esp = np->tf->esp - 8;
    copyout(np->pgdir, np->tf->esp, ustack, 8);

    //start_routine set main function in thread
    np->tf->eip = (uint)start_routine;

    acquire(&ptable.lock);

    np->state = RUNNABLE;

    release(&ptable.lock);

    return 0;
}


void 
thread_exit(void* retval)
{
    struct proc *curproc = myproc();
    int fd;

    if(curproc == initproc)
        panic("init exiting");

    for(fd = 0; fd < NOFILE; fd++){
        if(curproc->ofile[fd]){
            fileclose(curproc->ofile[fd]);
            curproc->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(curproc->cwd);
    end_op();

    curproc->cwd = 0;
    curproc->ret_val = (int)retval;
    acquire(&ptable.lock);

    wakeup1(curproc->parent);
    curproc->parent->numthread--;
    curproc->state = ZOMBIE;
    
    sched();
}

int
thread_join(thread_t thread, void** retval)
{
    struct proc *p;
    struct proc *curproc = myproc();

    acquire(&ptable.lock);
    for(;;){
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->threadid != thread)
                continue;

            if(p->parent != curproc)
                continue;
            if(p->state == ZOMBIE){
                *retval = (void*)p->ret_val;

                kfree(p->kstack);
                p->kstack = 0;
                //need to free page
                p->parent->usepage[p->havet] = 0;
                deallocuvm(p->pgdir, p->s_base + 2 * PGSIZE, p->s_base);
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                p->lwpon = 0;
                p->havet = 0;
                p->numthread = 0;
                p->threadid = 0;
                p->ret_val = 0;
                p->s_base = 0;
                
                p->state = UNUSED;
                release(&ptable.lock);
                return 0;
            }
        }

        sleep(curproc, &ptable.lock);  //DOC: wait-sleep
    }
}

