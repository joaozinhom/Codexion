#include <limits.h>
#include <stdio.h>
#include "../codexion.h"
#include "programmer.h"

static int	stop_if_all_finished(struct s_simulation *simulation)
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
	pthread_cond_broadcast(&simulation->state_changed);
	return (1);
}

static int	find_next_deadline(struct s_simulation *simulation,
		long *deadline)
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

static void	stop_after_burnout(struct s_simulation *simulation,
		int programmer_index, long deadline)
{
	simulation->stopped = 1;
	simulation->burnout_programmer_id
		= simulation->programmers[programmer_index].id;
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ldms %d burned out\n", deadline - simulation->start_time_ms,
		simulation->burnout_programmer_id);
	fflush(stdout);
	pthread_mutex_unlock(&simulation->log_mutex);
	pthread_cond_broadcast(&simulation->state_changed);
}

static void	wait_for_simulation_start(struct s_simulation *simulation)
{
	while (!simulation->started && !simulation->stopped)
		pthread_cond_wait(&simulation->state_changed,
			&simulation->state_mutex);
}

void	*monitor_burnout(void *context)
{
	struct s_simulation	*simulation;
	struct timespec		wait_until;
	long				deadline;
	int				programmer_index;

	simulation = context;
	pthread_mutex_lock(&simulation->state_mutex);
	wait_for_simulation_start(simulation);
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
