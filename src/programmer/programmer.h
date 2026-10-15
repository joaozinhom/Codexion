#ifndef PROGRAMMER_H
# define PROGRAMMER_H

# include <pthread.h>

struct	s_arguments;
struct	s_simulation;
struct	s_programmer;
struct	s_dongle;

typedef struct s_arguments	t_arguments;
typedef struct s_simulation	t_simulation;
typedef struct s_programmer	t_programmer;
typedef struct s_dongle		t_dongle;

enum e_programmer_state
{
	PROGRAMMER_WAITING,
	PROGRAMMER_COMPILING,
	PROGRAMMER_DEBUGGING,
	PROGRAMMER_REFACTORING,
	PROGRAMMER_FINISHED
};

typedef enum e_programmer_state	t_programmer_state;

struct s_dongle
{
	int			id;
	int			in_use;
	long		cooldown_deadline_ms;
};

struct s_programmer
{
	int					id;
	pthread_t			thread;
	t_simulation			*simulation;
	t_dongle				*left_dongle;
	t_dongle				*right_dongle;
	long					last_compile_start_ms;
	int					compiles_completed;
	t_programmer_state		state;
};

struct s_simulation
{
	const t_arguments			*arguments;
	t_programmer				*programmers;
	t_dongle					*dongles;
	pthread_t				monitor_thread;
	pthread_mutex_t				state_mutex;
	pthread_cond_t				state_changed;
	pthread_mutex_t				log_mutex;
	long					start_time_ms;
	int					started;
	int					stopped;
	int					burnout_programmer_id;
};

void	*monitor_burnout(void *context);

#endif