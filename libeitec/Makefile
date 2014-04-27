include ../config.mk

EXEC=lc_test
LIB=libeitec.a
RM=rm -f
OBJECTS=lc_byte.o lc_list.o lc_md5.o

all:$(LIB)

$(LIB):$(OBJECTS)
	$(AR) rcv $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

$(EXEC):lc_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) lc_test.o -leitec -L.


clean:
	$(RM) *.o *~ $(EXEC) $(LIB)

lc_byte.o: libeitec.h
lc_list.o: libeitec.h
lc_md5.o:  libeitec.h
