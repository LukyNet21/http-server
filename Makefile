CC=gcc
CFLAGS=-Wall -Wextra -std=c11
TARGET=server
SRCDIR=src
BUILDDIR=build

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(BUILDDIR)/main.o $(BUILDDIR)/server.o | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(TARGET) $(BUILDDIR)/main.o $(BUILDDIR)/server.o

$(BUILDDIR)/main.o: main.c $(SRCDIR)/server.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c main.c -o $(BUILDDIR)/main.o

$(BUILDDIR)/server.o: $(SRCDIR)/server.c $(SRCDIR)/server.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/server.c -o $(BUILDDIR)/server.o

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(BUILDDIR)/$(TARGET)
	./$(BUILDDIR)/$(TARGET)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean run
