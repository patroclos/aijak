#pragma once

#include <types.h>

#define PROCESS_STATE_ALIVE 0
#define PROCESS_STATE_ZOMBIE 1
#define PROCESS_STATE_DEAD 2

struct _process;

typedef struct _process
{
	struct _process *prev;
	char *           name;
	uint32           pid;
	uint32           esp;
	uint32           stacktop;
	uint32           eip;
	uint32           cr3;
	uint8           state;

	void (*notify)(int32);
	struct _process *next;
} PROCESS;

int addProcess(PROCESS *p);
PROCESS *createProcess(char *name, uint32 addr);

bool is_pid_running(uint32 pid);
bool is_tasking();

char *   p_name();
int      p_pid();
PROCESS *p_proc();

void send_sig(int sig);

void tasking_print_all();

void _sleep(uint32 ms);
void _kill();
void kill(uint32 pid);
void schedule();
void schedule_noirq();

void tasking_init(uint32 addr);

#define START(name, addr) addProcess(createProcess(name, (uint32)addr));
#define START_AND_WAIT(NAME, ADDR)          \
	int ADDR##_____pid = START(NAME, ADDR); \
	while (is_pid_running(ADDR##_____pid))  \
		schedule_noirq();
