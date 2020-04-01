nasm -f elf64 -w+all -w+error -o pix.o pix.asm
g++ -std=c++17 -Wall -Wextra -O2 -o test pix.o test.cpp -lpthread

