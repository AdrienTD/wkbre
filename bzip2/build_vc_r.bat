@cl -c -DWIN32 -MD -Ox -D_FILE_OFFSET_BITS=64 blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c
@lib /out:libbz2.lib *.obj