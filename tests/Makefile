TESTS=bitset routing_table merge
OBJECTS=tests.o test_bitset.o test_routing_table.o test_merge.o
INC_DIR=../include/
CFLAGS+=-I ${INC_DIR} -fprofile-arcs -ftest-coverage -g --std=gnu99
LDFLAGS+=-lcheck

coverage : run_tests
	gcov test_bitset test_routing_table test_merge

run_tests : tests
	valgrind --leak-check=full -q ./tests

tests : $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.c $(INC_DIR)/*.h
	$(CC) -c $(CFLAGS) $< -o $@


clean :
	$(RM) *.o *.gcov *.gcno *.gcda tests
