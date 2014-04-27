include ../config.mk

LIB=libtinyhttp.a
RM=rm -f
OBJECTS=chunk.o header.o http.o

all:$(LIB)

$(LIB):$(OBJECTS)
	$(AR) rcv $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

clean:
	$(RM) *.o *~ $(LIB)
