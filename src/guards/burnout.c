//the watchdog: one thread that only observes. it tracks each programmer's
//distance to burnout (last_compile_start + time_to_burnout), sleeps until
//the nearest deadline, and pulls the kill switch when it passes or when
//everyone reached number_of_compiles_required

#include <limits.h>
#include "../codexion.h"
#include "../programmer/programmer.h"

static int	stop_if_all_finished(t_simulation *simulation)
{
	int	i;

	i = 0;
	while (i < simulation->arguments->number_of_coders)
	{
		if (simulation->programmers[i].compiles_completed
			< simulation->arguments->number_of_compiles_required)
			return (0);
		i++;
	}
	simulation->stopped = 1;
	simulation_signal_stop(simulation);
	return (1);
}

static int	find_next_deadline(t_simulation *simulation, long *deadline)
{
	int		i;
	int		selected;
	long	candidate;

	i = 0;
	selected = -1;
	*deadline = LONG_MAX;
	while (i < simulation->arguments->number_of_coders)
	{
		if (simulation->programmers[i].compiles_completed
			< simulation->arguments->number_of_compiles_required)
		{
			candidate = simulation->programmers[i].last_compile_start_ms
				+ simulation->arguments->time_to_burnout;
			if (candidate < *deadline)
			{
				*deadline = candidate;
				selected = i;
			}
		}
		i++;
	}
	return (selected);
}

static void	stop_after_burnout(t_simulation *simulation,
		int programmer_index, long deadline)
{
	simulation->stopped = 1;
	simulation->burnout_programmer_id
		= simulation->programmers[programmer_index].id;
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ld %d burned out\n", deadline - simulation->start_time_ms,
		simulation->burnout_programmer_id);
	fflush(stdout);
	pthread_mutex_unlock(&simulation->log_mutex);
	simulation_signal_stop(simulation);
}

void	*monitor_burnout(void *context)
{
	t_simulation	*simulation;
	struct timespec	wait_until;
	long			deadline;
	int				programmer_index;

	simulation = context;
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->started && !simulation->stopped)
		pthread_cond_wait(&simulation->state_changed,
			&simulation->state_mutex);
	while (!simulation->stopped)
	{
		if (stop_if_all_finished(simulation))
			break ;
		programmer_index = find_next_deadline(simulation, &deadline);
		if (programmer_index >= 0 && current_time_ms() >= deadline)
			stop_after_burnout(simulation, programmer_index, deadline);
		else if (programmer_index >= 0)
		{
			wait_until = milliseconds_to_timespec(deadline);
			pthread_cond_timedwait(&simulation->state_changed,
				&simulation->state_mutex, &wait_until);
		}
	}
	pthread_mutex_unlock(&simulation->state_mutex);
	return (NULL);
}
