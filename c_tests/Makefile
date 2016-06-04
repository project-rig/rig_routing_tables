OBJECTS=tests.o test_bitset.o test_routing_table.o test_merge.o test_ordered_covering.o test_aliases.o test_mtrie.o test_remove_default_routes.o
INC_DIR=../include/
CFLAGS+=-I ${INC_DIR} -fprofile-arcs -ftest-coverage -g --std=gnu99 -Wall -Werror
LDFLAGS+=$(shell pkg-config --cflags --libs check)

coverage : run_tests
	gcov test_bitset test_routing_table test_merge test_aliases test_ordered_covering test_mtrie test_remove_default_routes

run_tests : tests
	valgrind --leak-check=full -q ./tests

tests : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o : %.c $(INC_DIR)/*.h
	$(CC) -c $(CFLAGS) $< -o $@


clean :
	$(RM) *.o *.gcov *.gcno *.gcda tests
