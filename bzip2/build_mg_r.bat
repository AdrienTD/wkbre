@del libbz2.a
@gcc -static -Wall -Winline -O2 -g -D_FILE_OFFSET_BITS=64 -c blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c
@ar cq libbz2.a *.o
@ranlib libbz2.a