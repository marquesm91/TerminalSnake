# Compiler, Flags, Directory Name and Executable Name
CC= g++
CFLAGS= -std=c++11
ODIR= ../obj

# Source codes and objects (Add here new .cpp tests separated by spaces)
SRCS= testPoint.cpp
EXEC= t1

OBJS= $(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

all: $(EXEC)

# Concatenate objects with your new directory
$(OBJS): | $(ODIR) 

# Compile and create objects
$(ODIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Generate the executables
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	rm -rf *.settings

# Delete objects, executables and new directories
clean:
	rm -rf $(OBJS) $(ODIR) $(EDIR)/* $(EDIR)