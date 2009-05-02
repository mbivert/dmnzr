CC = gcc
LD = $(CC)
CFLAGS = -ansi -Wall -W -O2 -pedantic -g
LIBS =
TARGET = dmnzr
OBJS = dmnzr.o xalloc.o

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

$(TARGET) : $(OBJS)
	$(LD) $(OBJS) -o $@

clean:
	@rm -vf $(OBJS) $(TARGET) *~
