@echo off

clang -Wall -std=c11 -O0 -g -gcodeview -march=native -mtune=native -D=DEBUG -Wall src/publican_win32.c -o pub.exe -luser32 -lgdi32 -lwinmm -lopengl32

