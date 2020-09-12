lmfs: main.o lmfs.o tape.o
	$(CC) $(CFLAGS) $^ -o $@
