psed: psed.c
	g++ psed.c -o psed -lpthread
tests:
	./test.sh $(RUN_ARGS)
