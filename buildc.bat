@echo off

gcc -Wall -g -c -std=c11 -O0 src/char_builder.c -o data/char_builder.o 
windres -i src/resource.rc -o data/resource.o
gcc data/resource.o data/char_builder.o -o charb -luser32 -lshell32 -lcomctl32 -lgdi32 -lole32 -lComdlg32