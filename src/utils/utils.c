#include <limits.h>
#include <sys/time.h>
#include "utils.h"

long	current_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

struct timespec	milliseconds_to_timespec(long milliseconds)
{
	struct timespec	time;

	time.tv_sec = milliseconds / 1000;
	time.tv_nsec = (milliseconds % 1000) * 1000000;
	return (time);
}

int	parse_nonnegative_int(const char *input, int *value)
{
	int	number;
	int	digit;

	if (!input || !*input)
		return (0);
	number = 0;
	while (*input)
	{
		if (*input < '0' || *input > '9')
			return (0);
		digit = *input - '0';
		if (number > (INT_MAX - digit) / 10)
			return (0);
		number = number * 10 + digit;
		input++;
	}
	*value = number;
	return (1);
}
