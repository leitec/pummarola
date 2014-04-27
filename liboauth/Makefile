include ../config.mk

EXEC=oauth_test
LIB=liboauth.a
CFLAGS+=-I. -I../polarssl/include -I.. -Wall
RM=rm -f
#CFLAGS+=-DOAUTH_DEBUG
LIBS=-leitec -loauth -lpolarssl
LDFLAGS=-L. -L../libeitec/ -L../polarssl/library/
OBJECTS=urlencode.o oauth.o kv.o strlcpy.o strlcat.o

all:$(LIB)

$(LIB):$(OBJECTS)
	$(AR) rcv $(LIB) $(OBJECTS)
	$(RANLIB) $(LIB)

$(EXEC):$(LIB) test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) test.o $(LIBS)

clean:
	$(RM) *.o *~ $(EXEC) $(LIB)
