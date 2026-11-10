
//the scheduler thread owns the request heap (fifo by arrival seq, edf by
//frozen deadline with seq tie-break), grants both dongles atomically, and
//signals the granted programmer's cond

#include <limits.h>
#include "../codexion.h"
#include "../programmer/programmer.h"

static int	request_earlier(t_scheduler *scheduler, t_request a, t_request b)
{
	if (scheduler->edf_mode)
	{
		if (a.deadline_ms != b.deadline_ms)
			return (a.deadline_ms < b.deadline_ms);
		return (a.seq < b.seq);
	}
	return (a.seq < b.seq);
}

static void	heap_push(t_scheduler *scheduler, t_request request)
{
	int	i;
	int	parent;

	i = scheduler->size++;
	scheduler->heap[i] = request;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!request_earlier(scheduler, scheduler->heap[i],
				scheduler->heap[parent]))
			break ;
		scheduler->heap[i] = scheduler->heap[parent];
		scheduler->heap[parent] = request;
		i = parent;
	}
}

static t_request	heap_pop(t_scheduler *scheduler)
{
	t_request	top;
	t_request	moved;
	int			i;
	int			child;

	top = scheduler->heap[0];
	moved = scheduler->heap[--scheduler->size];
	i = 0;
	while (2 * i + 1 < scheduler->size)
	{
		child = 2 * i + 1;
		if (child + 1 < scheduler->size && request_earlier(scheduler,
				scheduler->heap[child + 1], scheduler->heap[child]))
			child++;
		if (!request_earlier(scheduler, scheduler->heap[child], moved))
			break ;
		scheduler->heap[i] = scheduler->heap[child];
		i = child;
	}
	if (scheduler->size > 0)
		scheduler->heap[i] = moved;
	return (top);
}

void	request_dongles(t_simulation *simulation, int programmer_index)
{
	t_request	request;

	pthread_mutex_lock(&simulation->state_mutex);
	request.deadline_ms
		= simulation->programmers[programmer_index].last_compile_start_ms
		+ simulation->arguments->time_to_burnout;
	pthread_mutex_unlock(&simulation->state_mutex);
	request.programmer_index = programmer_index;
	pthread_mutex_lock(&simulation->scheduler.mutex);
	request.seq = simulation->scheduler.next_seq++;
	heap_push(&simulation->scheduler, request);
	pthread_cond_signal(&simulation->scheduler.cond);
	pthread_mutex_unlock(&simulation->scheduler.mutex);
}

static int	grant_dongles(t_programmer *programmer, long *wait_until_ms)
{
	long	now;
	int		granted;

	if (programmer->left_dongle == programmer->right_dongle)
		return (0);
	now = current_time_ms();
	*wait_until_ms = LONG_MAX;
	granted = 0;
	pthread_mutex_lock(&programmer->left_dongle->mutex);
	pthread_mutex_lock(&programmer->right_dongle->mutex);
	if (!programmer->left_dongle->in_use && !programmer->right_dongle->in_use
		&& now >= programmer->left_dongle->cooldown_deadline_ms
		&& now >= programmer->right_dongle->cooldown_deadline_ms)
	{
		programmer->left_dongle->in_use = 1;
		programmer->right_dongle->in_use = 1;
		granted = 1;
	}
	else
	{
		if (!programmer->left_dongle->in_use
			&& now < programmer->left_dongle->cooldown_deadline_ms)
			*wait_until_ms = programmer->left_dongle->cooldown_deadline_ms;
		if (!programmer->right_dongle->in_use
			&& now < programmer->right_dongle->cooldown_deadline_ms
			&& programmer->right_dongle->cooldown_deadline_ms
			< *wait_until_ms)
			*wait_until_ms = programmer->right_dongle->cooldown_deadline_ms;
	}
	pthread_mutex_unlock(&programmer->right_dongle->mutex);
	pthread_mutex_unlock(&programmer->left_dongle->mutex);
	return (granted);
}

static void	notify_grant(t_programmer *programmer)
{
	pthread_mutex_lock(&programmer->wait_mutex);
	programmer->granted = 1;
	pthread_cond_signal(&programmer->wait_cond);
	pthread_mutex_unlock(&programmer->wait_mutex);
}

void	*scheduler_routine(void *context)
{
	t_simulation	*simulation;
	t_programmer	*programmer;
	long			wait_until_ms;
	struct timespec	wait_until;

	simulation = context;
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->started && !simulation->stopped)
		pthread_cond_wait(&simulation->state_changed,
			&simulation->state_mutex);
	pthread_mutex_unlock(&simulation->state_mutex);
	pthread_mutex_lock(&simulation->scheduler.mutex);
	while (!simulation->scheduler.stop)
	{
		if (simulation->scheduler.size == 0)
		{
			pthread_cond_wait(&simulation->scheduler.cond,
				&simulation->scheduler.mutex);
			continue ;
		}
		programmer = &simulation->programmers[
			simulation->scheduler.heap[0].programmer_index];
		if (grant_dongles(programmer, &wait_until_ms))
		{
			heap_pop(&simulation->scheduler);
			notify_grant(programmer);
		}
		else if (wait_until_ms != LONG_MAX)
		{
			wait_until = milliseconds_to_timespec(wait_until_ms);
			pthread_cond_timedwait(&simulation->scheduler.cond,
				&simulation->scheduler.mutex, &wait_until);
		}
		else
			pthread_cond_wait(&simulation->scheduler.cond,
				&simulation->scheduler.mutex);
	}
	pthread_mutex_unlock(&simulation->scheduler.mutex);
	return (NULL);
}
