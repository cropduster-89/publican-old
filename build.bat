@echo off

gcc -std=gnu11 -O3 -ggdb2 -pg -march=native -D=DEBUG -Wall src/publican_win32.c -o pub.exe -luser32 -lgdi32 -lwinmm -lopengl32

