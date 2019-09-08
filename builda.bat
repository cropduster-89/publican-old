@echo off

gcc -g -O0 -std=c11 -Lc:/winbuilds src/asset_builder.c -o asset.exe -lgdi32