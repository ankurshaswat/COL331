// struct context {
//   uint edi;
//   uint esi;
//   uint ebx;
//   uint ebp;
//   uint eip;
// };

enum container_states
{
    UNUSED_CONTAINER,
    IN_USE
    // EMBRYO,
    // SLEEPING,
    // RUNNABLE,
    // RUNNING,
    // ZOMBIE
};

#define NUM_STORE_MAPS 20

struct process_table
{
    int local_id[NUM_STORE_MAPS];
    int global_id[NUM_STORE_MAPS];
};

// Per-container state
struct container
{
    // Using for assignment 3
    // int original_pid; // Process ID
    // int local_pid;
    int container_id;
    enum container_states state; // Process state
    struct process_table pmap;
    // Declared as copy of proc
    // uint sz;                    // Size of process memory (bytes)
    // pde_t *pgdir;               // Page table
    // char *kstack;               // Bottom of kernel stack for this process
    // struct proc *parent;        // Parent process
    // struct trapframe *tf;       // Trap frame for current syscall
    // struct context *context;    // swtch() here to run process
    // void *chan;                 // If non-zero, sleeping on chan
    // int killed;                 // If non-zero, have been killed
    // struct file *ofile[NOFILE]; // Open files
    // struct inode *cwd;          // Current directory
    // char name[16];              // Process name (debugging)
};