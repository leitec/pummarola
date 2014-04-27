include ../config.mk

LIB=libjson-parser.a
RM=rm -f
OBJECTS=json.o

all:$(LIB)

$(LIB):$(OBJECTS)
	$(AR) rcv $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

clean:
	$(RM) *.o *~ $(LIB)
