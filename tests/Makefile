OBJECTS=tests.o test_bitset.o
INC_DIR=../include/
CFLAGS+=-I ${INC_DIR} -fprofile-arcs -ftest-coverage -g
LDFLAGS+=-lcheck

coverage : run_tests
	gcov test_bitset.h

run_tests : tests
	valgrind --leak-check=full -q ./tests

tests : $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.c $(INC_DIR)/*.h
	$(CC) -c $(CFLAGS) $< -o $@


clean :
	$(RM) *.o tests
