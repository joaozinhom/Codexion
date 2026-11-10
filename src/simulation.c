#include <stdlib.h>
#include "codexion.h"
#include "programmer/programmer.h"

static void	init_dongles(t_simulation *simulation, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		simulation->dongles[i].id = i;
		simulation->dongles[i].in_use = 0;
		simulation->dongles[i].cooldown_deadline_ms = 0;
		pthread_mutex_init(&simulation->dongles[i].mutex, NULL);
		i++;
	}
}

static void	init_programmers(t_simulation *simulation,
		const t_arguments *arguments)
{
	int	i;

	i = 0;
	while (i < arguments->number_of_coders)
	{
		simulation->programmers[i].id = i + 1;
		simulation->programmers[i].simulation = simulation;
		simulation->programmers[i].left_dongle = &simulation->dongles[i];
		simulation->programmers[i].right_dongle
			= &simulation->dongles[(i + 1) % arguments->number_of_coders];
		simulation->programmers[i].last_compile_start_ms = 0;
		simulation->programmers[i].compiles_completed = 0;
		simulation->programmers[i].state = PROGRAMMER_WAITING;
		simulation->programmers[i].granted = 0;
		simulation->programmers[i].stop = 0;
		pthread_mutex_init(&simulation->programmers[i].wait_mutex, NULL);
		pthread_cond_init(&simulation->programmers[i].wait_cond, NULL);
		i++;
	}
}

int	simulation_init(t_simulation *simulation, const t_arguments *arguments)
{
	simulation->arguments = arguments;
	simulation->started = 0;
	simulation->stopped = 0;
	simulation->burnout_programmer_id = 0;
	simulation->start_time_ms = 0;
	simulation->programmers = malloc(sizeof(t_programmer)
			* arguments->number_of_coders);
	simulation->dongles = malloc(sizeof(t_dongle) * arguments->number_of_coders);
	simulation->scheduler.heap = malloc(sizeof(t_request)
			* arguments->number_of_coders);
	if (!simulation->programmers || !simulation->dongles
		|| !simulation->scheduler.heap)
		return (0);
	simulation->scheduler.size = 0;
	simulation->scheduler.capacity = arguments->number_of_coders;
	simulation->scheduler.edf_mode = (strcmp(arguments->scheduler, "edf") == 0);
	simulation->scheduler.stop = 0;
	simulation->scheduler.next_seq = 0;
	pthread_mutex_init(&simulation->scheduler.mutex, NULL);
	pthread_cond_init(&simulation->scheduler.cond, NULL);
	pthread_mutex_init(&simulation->state_mutex, NULL);
	pthread_cond_init(&simulation->state_changed, NULL);
	pthread_mutex_init(&simulation->log_mutex, NULL);
	init_dongles(simulation, arguments->number_of_coders);
	init_programmers(simulation, arguments);
	return (1);
}

void	simulation_run(t_simulation *simulation)
{
	int	i;
	int	count;

	count = simulation->arguments->number_of_coders;
	i = 0;
	while (i < count)
	{
		pthread_create(&simulation->programmers[i].thread, NULL,
			coder_routine, &simulation->programmers[i]);
		i++;
	}
	pthread_create(&simulation->scheduler_thread, NULL,
		scheduler_routine, simulation);
	pthread_create(&simulation->monitor_thread, NULL, monitor_burnout,
		simulation);
	pthread_mutex_lock(&simulation->state_mutex);
	simulation->start_time_ms = current_time_ms();
	i = 0;
	while (i < count)
	{
		simulation->programmers[i].last_compile_start_ms
			= simulation->start_time_ms;
		i++;
	}
	simulation->started = 1;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
	i = 0;
	while (i < count)
		pthread_join(simulation->programmers[i++].thread, NULL);
	pthread_join(simulation->scheduler_thread, NULL);
	pthread_join(simulation->monitor_thread, NULL);
}

void	simulation_signal_stop(t_simulation *simulation)
{
	int	i;

	pthread_mutex_lock(&simulation->scheduler.mutex);
	simulation->scheduler.stop = 1;
	pthread_cond_broadcast(&simulation->scheduler.cond);
	pthread_mutex_unlock(&simulation->scheduler.mutex);
	i = 0;
	while (i < simulation->arguments->number_of_coders)
	{
		pthread_mutex_lock(&simulation->programmers[i].wait_mutex);
		simulation->programmers[i].stop = 1;
		pthread_cond_broadcast(&simulation->programmers[i].wait_cond);
		pthread_mutex_unlock(&simulation->programmers[i].wait_mutex);
		i++;
	}
	pthread_cond_broadcast(&simulation->state_changed);
}

int	simulation_is_stopped(t_simulation *simulation)
{
	int	stopped;

	pthread_mutex_lock(&simulation->state_mutex);
	stopped = simulation->stopped;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (stopped);
}

void	simulation_destroy(t_simulation *simulation)
{
	int	i;

	i = 0;
	while (i < simulation->arguments->number_of_coders)
	{
		pthread_mutex_destroy(&simulation->programmers[i].wait_mutex);
		pthread_cond_destroy(&simulation->programmers[i].wait_cond);
		pthread_mutex_destroy(&simulation->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&simulation->scheduler.mutex);
	pthread_cond_destroy(&simulation->scheduler.cond);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_cond_destroy(&simulation->state_changed);
	pthread_mutex_destroy(&simulation->log_mutex);
	free(simulation->scheduler.heap);
	free(simulation->programmers);
	free(simulation->dongles);
}
