This file briefly describes how to use the provided makefile

* The "-B" option forces make to rebuild all targets. In the context
  of this lab, we strongly encourage you to always use this option.

* To generate the program mem_alloc_test with the default First Fit
  policy:
     $ make -B mem_alloc_test

* To generate the program mem_alloc_test and change the size of the
  memory zone to 65536:
     $ make -B -DMEMORY_SIZE=65536 mem_alloc_test

* To generate the program mem_shell with the Best Fit policy:
     $ make -B POLICY=BF mem_shell

* To check if your allocator passes test 2 with the Worst Fit policy:
     $ make -B POLICY=WF ./tests/alloc2.test

* To generate the output of mem_shell for test 4 with First Fit policy:
     $ make -B POLICY=FF ./tests/alloc4.out

* To generate the expected output for test 3 with First Fit policy:
     $ make -B POLICY=FF ./tests/alloc3.out.expected

* To clean all generated files:
     $ make clean
