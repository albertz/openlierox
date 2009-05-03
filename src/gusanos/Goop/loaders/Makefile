OBJS=gusanos.o liero.o lierox.o losp.o
CFLAGS=-I../../Console
DEFINES=

testrule: $(OBJS)
	echo "-- Built loaders"

.cpp.o:
	$(CC) -c $< $(CFLAGS) $(DEFINES)

