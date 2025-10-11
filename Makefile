NAME	= webserv
SRCDIR	= srcs
INCDIR	= include
SRCS = $(wildcard $(SRCDIR)/**/*.cpp) $(wildcard $(SRCDIR)/*.cpp)
OBJDIR  = objs
OBJS    = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.cpp=.o)))
CXX		= c++
CFLAGS	= -Wall -Werror -Wextra -I$(INCDIR) -g
CSTD	= -std=c++98

.DEFAULT:	all
all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(NAME): $(OBJDIR) $(OBJS)
	$(CXX) $(CFLAGS) $(CSTD) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) $(CSTD) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/*/%.cpp
	$(CXX) $(CFLAGS) $(CSTD) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
