@echo off

clang -Wall -std=c11 -O3 -g -gcodeview -march=native -D=DEBUG -Wall src/publican_win32.c -o pub.exe -luser32 -lgdi32 -lwinmm -lopengl32

