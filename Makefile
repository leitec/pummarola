include config.mk

EXEC=lp_test
LIBS=-loauth -lpummarola -ltinyhttp -ljson-parser -lpolarssl -lm -leitec
CFLAGS+=-I. -Ipolarssl/include
LDFLAGS=-Llibeitec/ -Lliboauth/ -Llibpummarola/ -Ltinyhttp/ -Ljson-parser -Lpolarssl/library
OBJECTS=lp_test.o

SUBDIRS=json-parser libeitec liboauth libpummarola tinyhttp

all:
	rm -f $(EXEC)

	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

	$(MAKE) $(EXEC)

$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) $(OBJECTS) $(LIBS)

clean:
	rm -f *.o *~ $(EXEC)

realclean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	$(MAKE) clean
