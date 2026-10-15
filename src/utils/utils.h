#ifndef UTILS_H
# define UTILS_H

# include <time.h>

int	parse_nonnegative_int(const char *input, int *value);

long	current_time_ms(void);

struct timespec	milliseconds_to_timespec(long milliseconds);

#endif
