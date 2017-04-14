#include <pit.h>

#include <display/display.h>
#include <interrupt/irq.h>
#include <tasking.h>

MODULE("PIT");

static uint8 task        = 0;
static bool  task_was_on = false;

uint32 time;

void set_task(uint8 i)
{
	if (!task_was_on)
		return;
	task = i;
}

void enable_task()
{
	task_was_on = true;
	task        = 1;
}

uint32 pit_gettime()
{
	return time;
}

void pit_irq(struct regs *r)
{
	time++;
	if (!task)
	{
		send_eoi(0);
	}
	else
	{
		schedule_noirq();
	}
}

static inline void __pit_send_cmd(uint8 cmd)
{
	outportb(PIT_REG_COMMAND, cmd);
}

static inline void __pit_send_data(uint16 data, uint8 counter)
{
	uint8 port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : ((counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);
	outportb(port, (uint8)data);
}

static inline uint8 __pit_read_data(uint16 counter)
{
	uint8 port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : ((counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);
	return inportb(port);
}

static void pit_start_counter(uint32 freq, uint8 counter, uint8 mode)
{
	if (!freq)
		return;
	mprint("Starting counter %d with frequency %dHz\n", counter / 0x40, freq);

	uint16 divisor = (uint16)(1193181 / (uint16)freq);

	// send operational command words
	uint8 ocw = 0;
	ocw       = (ocw & ~PIT_OCW_MASK_MODE) | mode;
	ocw       = (ocw & ~PIT_OCW_MASK_RL) | PIT_OCW_RL_DATA;
	ocw       = (ocw & ~PIT_OCW_MASK_COUNTER) | counter;
	__pit_send_cmd(ocw);

	// set frequency rate
	__pit_send_data(divisor & 0xff, 0);
	__pit_send_data((divisor >> 8) & 0xff, 0);
}

void pit_init()
{
	mprint("Registering IRQ#0=INT#32 as PIT_IRQ\n");
	irq_setroutine(0, (uint32)pit_irq);
	pit_start_counter(PIT_C0_HZ, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQAREWAVEGEN);
	mprint("Init done.\n");
}
