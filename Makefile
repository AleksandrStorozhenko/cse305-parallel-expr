APP ?= eval 
ifeq ($(APP),bench)
    TARGET       = bench
    EXCLUDE_SRCS = main.cpp
else
    TARGET       = eval
    EXCLUDE_SRCS = benchmark.cpp
endif

# Compiler and default flags

CXX = g++
CFLAGS = -pthread -std=c++14 -Wall
RELEASE_FLAGS = -pthread -std=c++14 -Wall -DNDEBUG -O2

# Directories
SRCDIR   = src
BUILDDIR = build

# Source and object files
SOURCES  = $(filter-out $(addprefix $(SRCDIR)/,$(EXCLUDE_SRCS)),\
                      $(wildcard $(SRCDIR)/*.cpp))
OBJECTS  = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: CFLAGS := $(CFLAGS)
all: $(TARGET)

fast: CFLAGS := $(RELEASE_FLAGS)
fast: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^

# Compile
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CFLAGS) -o $@ $<

# Other
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) eval bench
