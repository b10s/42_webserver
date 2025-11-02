NAME	= webserv
SRCDIR	= srcs
INCDIR	= includes
SRCS = $(wildcard $(SRCDIR)/**/*.cpp) $(wildcard $(SRCDIR)/*.cpp)
OBJDIR  = objs
OBJS    = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.cpp=.o)))
CXX		= c++
CFLAGS	= -Wall -Werror -Wextra -I$(INCDIR) -g -std=c++98 -pedantic

.DEFAULT:	all

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(NAME): $(OBJDIR) $(OBJS)
	$(CXX) $(CFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/*/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)
	rm -rf build

re: fclean all

format:
	find includes srcs tests -name *.cpp -o -name *.h -o -name *.hpp | xargs clang-format -i -style=file --verbose

test:
	cmake -S . -B ./build
	cmake --build ./build
	cd build && ctest

.PHONY: all clean fclean re format test
