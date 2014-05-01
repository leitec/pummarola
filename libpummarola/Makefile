include ../config.mk

LIB=libpummarola.a
RM=rm -f
OBJECTS=auth.o http.o json.o libpummarola.o tweet.o url.o user.o

CFLAGS+=-I.. -I../polarssl/include

all:$(LIB)

$(LIB):$(OBJECTS)
	$(AR) rcv $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

clean:
	$(RM) *.o *~ $(LIB)
