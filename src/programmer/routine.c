#include "codexion.h"
#include "programmer/programmer.h"

static void	work_phase(t_programmer *programmer, t_programmer_state state,
		const char *event, long duration_ms)
{
	t_simulation	*simulation;

	simulation = programmer->simulation;
	set_programmer_state(simulation, programmer, state);
	log_event(simulation, programmer->id, event);
	sleep_ms(duration_ms);
}

void	*coder_routine(void *context)
{
	t_programmer	*programmer;
	t_simulation	*simulation;

	programmer = context;
	simulation = programmer->simulation;
	coder_wait_start(simulation);
	while (!simulation_is_stopped(simulation)
		&& compiles_done(simulation, programmer)
		< simulation->arguments->number_of_compiles_required)
	{
		set_programmer_state(simulation, programmer, PROGRAMMER_WAITING);
		request_dongles(simulation, programmer->id - 1);
		if (!wait_for_grant(programmer))
			break ;
		log_event(simulation, programmer->id, "has taken a dongle");
		log_event(simulation, programmer->id, "has taken a dongle");
		set_compile_started(simulation, programmer);
		log_event(simulation, programmer->id, "is compiling");
		sleep_ms(simulation->arguments->time_to_compile);
		complete_compile(simulation, programmer);
		release_dongles(simulation, programmer);
		if (simulation_is_stopped(simulation))
			break ;
		work_phase(programmer, PROGRAMMER_DEBUGGING, "is debugging",
			simulation->arguments->time_to_debug);
		if (simulation_is_stopped(simulation))
			break ;
		work_phase(programmer, PROGRAMMER_REFACTORING, "is refactoring",
			simulation->arguments->time_to_refactor);
	}
	set_programmer_state(simulation, programmer, PROGRAMMER_FINISHED);
	return (NULL);
}
