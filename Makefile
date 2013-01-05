CC = cc
CFLAGS = -ansi -Wall -W -O2 -pedantic -g
ROFF=/usr/bin/nroff
TARGET = dmnzr
OBJS = dmnzr.o

all: $(TARGET) $(TARGET).cat8

%.cat8: %.8
	$(ROFF) -man $< > $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	@rm -rf $(OBJS) $(TARGET) $(TARGET).cat8 *~
