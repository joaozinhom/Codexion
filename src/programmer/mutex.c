//the waiting primitives: a coder sleeps on his own wait_cond until the
//scheduler grants the dongles, and every thread crosses the start barrier
//before acting, so all timelines share the same t=0

#include "../codexion.h"
#include "programmer.h"

void	coder_wait_start(t_simulation *simulation)
{
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->started && !simulation->stopped)
		pthread_cond_wait(&simulation->state_changed,
			&simulation->state_mutex);
	pthread_mutex_unlock(&simulation->state_mutex);
}

int	wait_for_grant(t_programmer *programmer)
{
	int	granted;

	pthread_mutex_lock(&programmer->wait_mutex);
	while (!programmer->granted && !programmer->stop)
		pthread_cond_wait(&programmer->wait_cond, &programmer->wait_mutex);
	granted = programmer->granted;
	programmer->granted = 0;
	pthread_mutex_unlock(&programmer->wait_mutex);
	return (granted && !simulation_is_stopped(programmer->simulation));
}
