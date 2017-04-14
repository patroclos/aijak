#include <tasking.h>

#include <display/display.h>
#include <interrupt/idt.h>
#include <memory/memory.h>
#include <signal.h>

MODULE("TASK");

PROCESS *c         = 0;
uint32   lpid      = 0;
bool     __enabled = false;

static void (*__late_init)();

void task1()
{
	mprint("Tasking online.\n");
	_kill();
}

void task2()
{
	while (true)
		kprintf("[2] Hello, I am task 2! :))\n");
}

void task3()
{
	while (true)
		kprintf("[3] watch my mutex game\n");
}

void idle_thread()
{
	enable_task();
	__enabled = true;
	if (__late_init)
		__late_init();
}

void kill(uint32 pid)
{
	if (pid == 1)
		panic("Idle cant be killed!\n");
	if (pid == c->pid)
		_kill();
	PROCESS *orig = c;
	PROCESS *p    = orig;
	while (true)
	{
		if (p->pid == pid)
		{
			mprint("Process %s (%d) was set to ZOMBIE.\n", p->name, pid);
			p->state = PROCESS_STATE_ZOMBIE;
			break;
		}
		p = p->next;
		if (p == orig)
			break;
	}
}

void send_sig(int sig)
{
	c->notify(sig);
}

bool is_tasking()
{
	return __enabled;
}

PROCESS *p_proc()
{
	return c;
}

char *p_name()
{
	return c->name;
}

int32 p_pid()
{
	return c->pid;
}

void _kill()
{
	if (c->pid == 1)
	{
		set_task(0);
		panic("Idle cant be killed!");
	}
	mprint("Killing process %s (%d)\n", c->name, c->pid);
	set_task(0);
	free((void *)c->stacktop);
	free(c);
	pfree((void *)c->cr3);
	c->prev->next = c->next;
	c->next->prev = c->prev;
	set_task(1);
	schedule_noirq();
}

void _sleep(uint32 ms)
{
	uint32 ticks      = PIT_C0_HZ / 1000.0f * ms;
	uint32 wakeuptime = pit_gettime() + ticks;
	while (pit_gettime() < wakeuptime)
		schedule_noirq();
}

// Jack, the ripport.
// killing zombies is what he does

void jack_the_ripper()
{
reset:;
	PROCESS *orig = c;
	PROCESS *p    = orig;
	while (true)
	{
		p = p->next;
		if (p == c)
			continue;
		if (p->state == PROCESS_STATE_ZOMBIE)
		{
			set_task(0);
			p->prev->next = p->next;
			p->next->prev = p->prev;
			free((void *)p->stacktop);
			free(p);
			set_task(1);
			mprint("Jack has reaped yet another one: %s (%d)\n", p->name, p->pid);
		}
		if (p == orig)
			goto reset;
		schedule_noirq();
	}
}

void tasking_print_all()
{
	PROCESS *orig = c;
	PROCESS *p    = orig;
	while (true)
	{
		kprintf("Process: %s (%d) %s\n", p->name, p->pid, p->state == PROCESS_STATE_ZOMBIE ? "ZOMBIE" : p->state == PROCESS_STATE_ALIVE ? "ALIVE" : "DEAD");
		p = p->next;
		if (p == orig)
			break;
	}
}

void __notified(int sig)
{
	switch (sig)
	{
	case SIGILL:
		kprintf("Task #%d received SIGILL, terminating!\n", p_pid());
		_kill();
		break;
	case SIGTERM:
		kprintf("Task #%d received SIGTERM, terminating!\n", p_pid());
		_kill();
		break;
	case SIGSEGV:
		kprintf("Task #%d received SIGSEGV, terminating!\n", p_pid());
		_kill();
		break;
	default:
		kprintf("Task #%d received an unknown signal(%d)\n", p_pid(), sig);
		break;
	}
}

bool is_pid_running(uint32 pid)
{
	set_task(0);
	PROCESS *p    = c;
	PROCESS *orig = c;
	bool     ret  = false;
	while (true)
	{
		if (p->pid == pid)
		{
			ret = true;
			break;
		}
		p = p->next;
		if (p == orig)
			break;
	}
	set_task(1);
	return ret;
}

PROCESS *createProcess(char *name, uint32 addr)
{
	PROCESS *p = (PROCESS *)malloc(sizeof(PROCESS));
	memset(p, 0, sizeof(PROCESS));
	p->name   = name;
	p->pid    = ++lpid;
	p->eip    = addr;
	p->state  = PROCESS_STATE_ALIVE;
	p->notify = __notified;
	p->esp    = (uint32)malloc(4096);
	asm volatile("mov %%cr3, %%eax"
	             : "=a"(p->cr3));
	uint32 *stack = (uint32 *)(p->esp + 4096);
	p->stacktop   = p->esp;
	*--stack      = 0x00000202; // eflags
	*--stack      = 0x8; // cs
	*--stack      = (uint32)addr; //eip
	*--stack      = 0; // eax
	*--stack      = 0; // ebx
	*--stack      = 0; // ecx
	*--stack      = 0; // edx
	*--stack      = 0; // esi
	*--stack      = 0; // edi
	*--stack      = p->esp + 4096; // ebp
	*--stack      = 0x10; // ds
	*--stack      = 0x10; // fs
	*--stack      = 0x10; // es
	*--stack      = 0x10; // gs
	p->esp        = (uint32)stack;
	mprint("Created task %s (%d) with esp=0x%x eip=0x%x\n", p->name, p->pid, p->esp, p->eip);
	return p;
}

// adds a process while no others are running!
void __addProcess(PROCESS *p)
{
	p->next       = c->next;
	p->next->prev = p;
	p->prev       = c;
	c->next       = p;
}

// adds process but take care of thers also
int addProcess(PROCESS *p)
{
	set_task(0);
	__addProcess(p);
	set_task(1);
	return p->pid;
}

// starts tasking
void __exec()
{
	asm volatile("mov %%eax, %%esp"
	             :
	             : "a"(c->esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}

void schedule_noirq()
{
	if (!__enabled)
		return;
	asm volatile("int $0x79");
}

void schedule()
{
	asm volatile("push %eax");
	asm volatile("push %ebx");
	asm volatile("push %ecx");
	asm volatile("push %edx");
	asm volatile("push %esi");
	asm volatile("push %ebp");
	asm volatile("push %ds");
	asm volatile("push %es");
	asm volatile("push %fs");
	asm volatile("push %gs");
	asm volatile("mov %%esp, %%eax"
	             : "=a"(c->esp));

	c = c->next;

	asm volatile("mov %%eax, %%cr3"
	             :
	             : "a"(c->cr3));
	asm volatile("mov %%eax, %%esp"
	             :
	             : "a"(c->esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	asm volatile("out %%al, %%dx"
	             :
	             : "d"(0x20), "a"(0x20)); // send eoi to maste pic
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}

void tasking_init(uint32 addr)
{
	__late_init = (void (*)())addr;
	mprint("Creating idle process\n");
	c       = createProcess("kIdle", (uint32)idle_thread);
	c->next = c;
	c->prev = c;
	__addProcess(createProcess("task1", (uint32)task1));
	//__addProcess(createProcess("task2", (uint32)task2));
	//__addProcess(createProcess("task3", (uint32)task3));
	set_idt_gate(0x79, (uint32)schedule);

	__exec();
	panic("Failed to start tasking!");
}
