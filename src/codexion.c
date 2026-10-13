#include "codexion.h"

static void	set_arguments(struct s_arguments *args, int *values,
		char *scheduler)
{
	args->number_of_coders = values[0];
	args->time_to_burnout = values[1];
	args->time_to_compile = values[2];
	args->time_to_debug = values[3];
	args->time_to_refactor = values[4];
	args->number_of_compiles_required = values[5];
	args->dongle_cooldown = values[6];
	args->scheduler = scheduler;
}

static int	parse_arguments(struct s_arguments *args, char **argv)
{
	int	values[7];
	int	i;

	i = 0;
	while (i < 7)
	{
		if (!parse_nonnegative_int(argv[i + 1], &values[i]))
			return (fprintf(stderr, "Invalid numeric argument: %s\n",
					argv[i + 1]), 0);
		i++;
	}
	if (values[0] == 0)
		return (fprintf(stderr,
				"number_of_coders must be greater than 0\n"), 0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (fprintf(stderr, "scheduler must be fifo or edf\n"), 0);
	set_arguments(args, values, argv[8]);
	return (1);
}

int	main(int argc, char **argv)
{
	struct s_arguments	args;

	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n", argv[0]);
		return (1);
	}
	if (!parse_arguments(&args, argv))
		return (1);
	return (0);
}
