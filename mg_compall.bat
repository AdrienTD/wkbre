@windres wkbre.rc res.o
@gcc %1 %2 %3 %4 %5 %6 %7 %8 %9 -c *.c *.cpp gameset/*.cpp play/*.cpp script/*.cpp -Wno-write-strings -fpermissive
@g++ %1 %2 %3 %4 %5 %6 %7 %8 %9 -g -o wkbre_mg.exe *.o -ld3d9 -lgdi32 -lwinmm