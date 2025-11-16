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

CLANG_IMAGE := silkeh/clang:19

CLANG_DOCKER := docker run --rm -v $$(pwd):/app -w /app $(CLANG_IMAGE) /bin/sh -c

format:
	$(CLANG_DOCKER)	\
	"find includes srcs -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i -style=file --verbose"

tidy:
	$(CLANG_DOCKER) \
	"find includes srcs -name *.cpp -o -name *.h -o -name *.hpp | xargs -I {} clang-tidy --config-file=.clang-tidy {} -- -Iincludes -std=c++98"

tidy-fix:
	$(CLANG_DOCKER) \
	"find includes srcs -name *.cpp -o -name *.h -o -name *.hpp | xargs -I {} clang-tidy --config-file=.clang-tidy -fix-errors {} -- -Iincludes -std=c++98"

test:
	cmake -S . -B ./build
	cmake --build ./build
	cd build && ctest

.PHONY: all clean fclean re format test
