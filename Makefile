NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -Isrc -Isrc/programmer -Isrc/utils

SRCS		= src/codexion.c \
			  src/simulation.c \
			  src/guards/burnout.c \
			  src/guards/scheduler.c \
			  src/programmer/routine.c \
			  src/programmer/programmer_state.c \
			  src/programmer/dongles.c \
			  src/programmer/mutex.c \
			  src/utils/utils.c

OBJS		= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
