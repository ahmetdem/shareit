# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++17

# Source files directories
SRCDIR = ./src
INCDIR = ./include

# Object files directories
OBJDIR = obj

# Output directories
OUTDIR = bin

# Source files
SERVER_SRCS := $(wildcard $(SRCDIR)/Server.cpp)
SERVER_SRCS += $(wildcard $(SRCDIR)/serverMain.cpp)
CLIENT_SRCS := $(wildcard $(SRCDIR)/Client.cpp)
CLIENT_SRCS += $(wildcard $(SRCDIR)/clientMain.cpp)
SERVER_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CLIENT_SRCS))

# Executables
SERVER_TARGET = $(OUTDIR)/server
CLIENT_TARGET = $(OUTDIR)/client

.PHONY: all clean run-server run-client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $(CLIENT_TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@rm -rf $(OBJDIR) $(OUTDIR)
