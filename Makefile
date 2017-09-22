CC=gcc
PEDANTIC_PARANOID_FREAK =       -g -O0 -Wall -Wshadow -Wcast-align \
				-Waggregate-return -Wstrict-prototypes \
				-Wredundant-decls -Wnested-externs \
				-Wpointer-arith -Wwrite-strings -finline-functions -Werror
REASONABLY_CAREFUL_DUDE =	-Wall
NO_PRAYER_FOR_THE_WICKED =	-w
WARNINGS = 			$(PEDANTIC_PARANOID_FREAK)

CFLAGS =   -g  $(WARNINGS)
LDFLAGS= 

ifeq ($(POLICY), BF)
$(info Using Best Fit policy)
CONFIG_FLAG= -DBEST_FIT
else ifeq ($(POLICY), WF)
$(info Using Worst Fit policy)
CONFIG_FLAG= -DWORST_FIT
else
$(info Using First Fit policy)
CONFIG_FLAG= -DFIRST_FIT
endif


TEST_FILES = $(wildcard tests/*.in)
TEST_LIST = $(patsubst %.in,%.test,$(TEST_FILES))
TEST_LIST += test_leak.test test_leak2.test

BIN_FILES = mem_alloc_test mem_shell mem_shell_sim leak_test leak_test2

mem_alloc_test:mem_alloc.c mem_alloc_types.h
	$(CC) -DMAIN -DMEMORY_SIZE=2048 $(CONFIG_FLAG) $(CFLAGS) $(LDFLAGS) $< -o $@

mem_shell:mem_shell.c mem_alloc.c mem_alloc_types.h
	$(CC) -DMEMORY_SIZE=512 $(CONFIG_FLAG) $(CFLAGS) $(LDFLAGS) $^ -o $@

mem_shell_sim:mem_shell.c mem_alloc_sim.c mem_alloc_types.h
	$(CC) -DMEMORY_SIZE=512 $(CONFIG_FLAG) $(CFLAGS) $(LDFLAGS) $^ -o $@


libmalloc.o:mem_alloc.c mem_alloc_types.h
	$(CC) -DMEMORY_SIZE=8388608 $(CONFIG_FLAG) $(CFLAGS) -fPIC -c $< -o $@
#	$(CC) -DMEMORY_SIZE=65536 $(CONFIG_FLAG) $(CFLAGS) -fPIC -c $< -o $@

libmalloc_std.o:mem_alloc_std.c mem_alloc_types.h
	$(CC) -DMEMORY_SIZE=8388608 $(CONFIG_FLAG) $(CFLAGS) -fPIC -c $< -o $@

libmalloc.so:libmalloc.o libmalloc_std.o
	$(CC)  -shared -Wl,-soname,$@ $^ -o $@

test_ls:libmalloc.so
	LD_PRELOAD=./libmalloc.so ls
	LD_PRELOAD=""

leak_test: tests/leak_test.c libmalloc.o libmalloc_std.o 
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

leak_test2: tests/leak_test2.c mem_alloc.c
	$(CC) -DMEMORY_SIZE=1024 $(CONFIG_FLAG) $(CFLAGS) $(LDFLAGS) $^ -o $@


%.out: %.in ./mem_shell
	@cat $< | ./mem_shell >/dev/null 2>$@

%.out.expected: %.in ./mem_shell_sim
	@cat $< | ./mem_shell_sim >/dev/null 2>$@

%.test: %.out %.out.expected
	@if diff $^  >/dev/null; then \
	  echo "**** Test $@ Passed *****"; \
	else \
	  echo "**** Test $@ FAILED *****"; \
	  diff -y $^ ;\
	fi

test_leak.out: leak_test ./libmalloc.so
	@LD_PRELOAD=./libmalloc.so ./$< > /dev/null 2>$@
	@LD_PRELOAD=""

test_leak2.out: leak_test ./libmalloc.so
	@LD_PRELOAD=./libmalloc.so ./$< do_not_leak > /dev/null 2>$@
	@LD_PRELOAD=""

test: $(TEST_LIST)

clean:
	rm -f $(BIN_FILES) *.o *~ tests/*~ tests/*.out tests/*.expected *.so mem_alloc_test

.PHONY: clean test
