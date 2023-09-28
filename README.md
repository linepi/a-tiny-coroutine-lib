# A tiny coroutine lib

## how to build
In the root folder:
```
make
```

## how to test
In the root folder:
```
make test
```

When you see output like this:
```
Test #1. Expect: (X|Y){0, 1, 2, ..., 199}
Y0  X1  Y2  Y3  Y4  X5  X6  Y7  X8  X9  X10  X11  X12...

Test #2. Expect: (libco-){200, 201, 202, ..., 399}
libco-200  libco-201  libco-202  libco-203  libco-204  libco-205  libco-206  libco-207  libco-208  libco-209  libco-210  libco-211...  
```
You've got a very simple but usable coroutine library!

## how to play
You can see this line in the Makefile in root folder:
```
TEST_TARGET = main
```
Write a new C file in tests folder(like yourprogram.c), change "main" to the C file name(like yourprogram) and then make test. 

You will see how your playing program running!