//the dongle is a passive shared resource: releasing resets in_use and arms
//the cooldown timestamp, then wakes the scheduler so the next request can
//be tried

#include "../codexion.h"
#include "programmer.h"

static void	release_one(t_dongle *dongle, long cooldown_deadline_ms)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->cooldown_deadline_ms = cooldown_deadline_ms;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_simulation *simulation, t_programmer *programmer)
{
	long	cooldown_deadline_ms;

	if (programmer->left_dongle == programmer->right_dongle)
		return ;
	cooldown_deadline_ms = current_time_ms()
		+ simulation->arguments->dongle_cooldown;
	release_one(programmer->left_dongle, cooldown_deadline_ms);
	release_one(programmer->right_dongle, cooldown_deadline_ms);
	pthread_mutex_lock(&simulation->scheduler.mutex);
	pthread_cond_broadcast(&simulation->scheduler.cond);
	pthread_mutex_unlock(&simulation->scheduler.mutex);
}
