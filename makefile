# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Target binaries
TARGETS = TCP_Sender TCP_Receiver RUDP_Sender RUDP_Receiver

# Source files
SRCS = TCP_Sender.c TCP_Receiver.c RUDP_API.c RUDP_Sender.c RUDP_Receiver.c

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

# Rule to build RUDP_Sender
RUDP_Sender: RUDP_API.o RUDP_Sender.o
	$(CC) $(CFLAGS) -o RUDP_Sender RUDP_API.o RUDP_Sender.o

# Rule to build RUDP_Receiver
RUDP_Receiver: RUDP_API.o RUDP_Receiver.o
	$(CC) $(CFLAGS) -o RUDP_Receiver RUDP_API.o RUDP_Receiver.o

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGETS)

# Phony targets
.PHONY: all clean
