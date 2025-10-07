NAME	= webserv
SRCDIR	= srcs
INCDIR	= includes
SRCS	= $(notdir $(wildcard $(SRCDIR)/*.cpp))
OBJDIR  = objs
OBJS    = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
CC		= c++
CFLAGS	= -Wall -Werror -Wextra -I$(INCDIR)
CSTD	= -std=c++98

.DEFAULT:	all
all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(NAME): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) $(CSTD) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CSTD) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
