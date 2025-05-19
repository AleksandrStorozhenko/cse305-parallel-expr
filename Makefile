# Compiler and default flags
CXX = g++
CFLAGS = -pthread -std=c++14 -Wall
RELEASE_FLAGS = -pthread -std=c++14 -Wall -DNDEBUG -O2

# Directories
SRCDIR = src
BUILDDIR = build
TARGET = eval

# Source and object files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Default target
all: CFLAGS := $(CFLAGS)
all: $(TARGET)

# Fast (release) build
fast: CFLAGS := $(RELEASE_FLAGS)
fast: $(TARGET)

# Link the final binary
$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^

# Compile source files to object files in build/
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) -c $(CFLAGS) -o $@ $<

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean up
clean:
	rm -rf $(BUILDDIR) $(TARGET)
