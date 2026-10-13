#include <limits.h>
#include "utils.h"

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
