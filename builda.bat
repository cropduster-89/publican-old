@echo off

clang -g -gcodeview -O3 -std=c11 -Lc:/winbuilds src/asset_builder.c -o asset.exe -lgdi32