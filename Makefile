CFLAGS = -g -Wformat-security -Wall -Wextra -Werror -ansi
LDFLAGS = -lpthread
CC = cc
LD = cc

TARG = a7

OBJS = a7.o

$(TARG): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(TARG)

clean:
	rm -f $(OBJS) $(TARG) core a.out
