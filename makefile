# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Target binaries
TARGETS = TCP_Sender TCP_Receiver

# Source files
SRCS = TCP_Sender.c TCP_Receiver.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGETS)

# Rule to build TCP_Sender
TCP_Sender: TCP_Sender.o
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.o

# Rule to build TCP_Receiver
TCP_Receiver: TCP_Receiver.o
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.o

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGETS)

# Phony targets
.PHONY: all clean
