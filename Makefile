# Compiler and flags
CXX = g++
CFLAGS = -pthread -std=c++20 -Wall -g -O0 -fsanitize=address
RELEASE_FLAGS = -pthread -std=c++17 -Wall -DNDEBUG -O2

# Directories
SRCDIR   = src
BUILDDIR = build

# All source files
ALL_SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Targets
all: clean eval bench

# eval target (excludes benchmark.cpp)
EVAL_SRCS   = $(filter-out $(SRCDIR)/benchmark.cpp,$(ALL_SOURCES))
EVAL_OBJS   = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/eval_%.o,$(EVAL_SRCS))
eval: CFLAGS := $(CFLAGS)
eval: $(EVAL_OBJS)
	$(CXX) $(CFLAGS) -o $@ $^

# bench target (excludes main.cpp)
BENCH_SRCS  = $(filter-out $(SRCDIR)/main.cpp,$(ALL_SOURCES))
BENCH_OBJS  = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/bench_%.o,$(BENCH_SRCS))
bench: CFLAGS := $(CFLAGS)
bench: $(BENCH_OBJS)
	$(CXX) $(CFLAGS) -o $@ $^

# Compile rules
$(BUILDDIR)/eval_%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/bench_%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CFLAGS) -o $@ $<

# Ensure build dir exists
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean
clean:
	rm -rf $(BUILDDIR) eval bench
