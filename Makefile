# Compiler, Flags, Directory Name and Executable Name
CC= g++
CFLAGS= -std=c++11
EFLAGS= -lncurses
ODIR= ./obj
EDIR= ./bin
EXEC1= tsnake

# Source codes and objects
SRCS= main.cpp
OBJS= $(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

all: $(EDIR)/$(EXEC1)

# Create paste for Objects
$(ODIR):
	@mkdir -p $@

# Concatenate objects with your new directory
$(OBJS): | $(ODIR)

# Special dependencies
#main.o: t2048_Linux.h

# Compile and create objects
$(ODIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(EFLAGS)

# Generate the executables
$(EDIR)/$(EXEC1): $(OBJS)
	@mkdir -p $(EDIR) # Create bin paste
	$(CC) $(CFLAGS) $^ -o $@ $(EFLAGS)
	rm -rf *.settings

# Delete objects, executables and new directories
clean:
	rm -rf $(OBJS) $(ODIR) $(EDIR)/* $(EDIR)