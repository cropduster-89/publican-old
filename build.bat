@echo off

clang -std=c11 -O0 -g -gcodeview -march=native -D=DEBUG -Wall src/publican_win32.c -o pub.exe -luser32 -lgdi32 -lwinmm -lopengl32

