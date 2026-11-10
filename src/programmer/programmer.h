#ifndef PROGRAMMER_H
# define PROGRAMMER_H

# include <pthread.h>

struct	s_arguments;

typedef struct s_arguments	t_arguments;

enum e_programmer_state
{
	PROGRAMMER_WAITING,
	PROGRAMMER_COMPILING,
	PROGRAMMER_DEBUGGING,
	PROGRAMMER_REFACTORING,
	PROGRAMMER_FINISHED
};

typedef enum e_programmer_state	t_programmer_state;

typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			cooldown_deadline_ms;
	pthread_mutex_t	mutex;
}	t_dongle;

typedef struct s_programmer
{
	int					id;
	pthread_t			thread;
	struct s_simulation	*simulation;
	t_dongle			*left_dongle;
	t_dongle			*right_dongle;
	long				last_compile_start_ms;
	int					compiles_completed;
	t_programmer_state	state;
	int					granted;
	int					stop;
	pthread_mutex_t		wait_mutex;
	pthread_cond_t		wait_cond;
}	t_programmer;

typedef struct s_request
{
	int				programmer_index;
	long			deadline_ms;
	unsigned long	seq;
}	t_request;

typedef struct s_scheduler
{
	t_request		*heap;
	int				size;
	int				capacity;
	int				edf_mode;
	int				stop;
	unsigned long	next_seq;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
}	t_scheduler;

struct s_simulation
{
	const t_arguments	*arguments;
	t_programmer		*programmers;
	t_dongle			*dongles;
	t_scheduler			scheduler;
	pthread_t			monitor_thread;
	pthread_t			scheduler_thread;
	pthread_mutex_t		state_mutex;
	pthread_cond_t		state_changed;
	pthread_mutex_t		log_mutex;
	long				start_time_ms;
	int					started;
	int					stopped;
	int					burnout_programmer_id;
};

typedef struct s_simulation	t_simulation;

int		simulation_init(t_simulation *simulation, const t_arguments *arguments);
void	simulation_run(t_simulation *simulation);
void	simulation_destroy(t_simulation *simulation);
void	simulation_signal_stop(t_simulation *simulation);
int		simulation_is_stopped(t_simulation *simulation);

void	*coder_routine(void *context);
void	*scheduler_routine(void *context);
void	*monitor_burnout(void *context);

void	request_dongles(t_simulation *simulation, int programmer_index);
void	release_dongles(t_simulation *simulation, t_programmer *programmer);

void	coder_wait_start(t_simulation *simulation);
int		wait_for_grant(t_programmer *programmer);

void	set_programmer_state(t_simulation *simulation, t_programmer *programmer,
			t_programmer_state state);
void	set_compile_started(t_simulation *simulation, t_programmer *programmer);
void	complete_compile(t_simulation *simulation, t_programmer *programmer);
int		compiles_done(t_simulation *simulation, t_programmer *programmer);
void	log_event(t_simulation *simulation, int programmer_id, const char *event);

#endif
