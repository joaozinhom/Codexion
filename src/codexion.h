#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <string.h>
# include "utils.h"

struct s_arguments
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	const char	*scheduler;
};

#endif
