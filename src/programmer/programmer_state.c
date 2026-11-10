//one writer per piece of state: only these functions mutate a programmer's
//shared fields, always under state_mutex, so observers (the watchdog, the
//scheduler) always read a consistent value

#include "../codexion.h"
#include "programmer.h"

void	set_programmer_state(t_simulation *simulation,
		t_programmer *programmer, t_programmer_state state)
{
	pthread_mutex_lock(&simulation->state_mutex);
	programmer->state = state;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
}

void	set_compile_started(t_simulation *simulation, t_programmer *programmer)
{
	pthread_mutex_lock(&simulation->state_mutex);
	programmer->last_compile_start_ms = current_time_ms();
	programmer->state = PROGRAMMER_COMPILING;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
}

void	complete_compile(t_simulation *simulation, t_programmer *programmer)
{
	pthread_mutex_lock(&simulation->state_mutex);
	programmer->compiles_completed++;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
}

int	compiles_done(t_simulation *simulation, t_programmer *programmer)
{
	int	completed;

	pthread_mutex_lock(&simulation->state_mutex);
	completed = programmer->compiles_completed;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (completed);
}

void	log_event(t_simulation *simulation, int programmer_id, const char *event)
{
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ld %d %s\n", current_time_ms() - simulation->start_time_ms,
		programmer_id, event);
	fflush(stdout);
	pthread_mutex_unlock(&simulation->log_mutex);
}
