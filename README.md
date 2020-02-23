# Traffic-Threading

Running environment: Cygwin

Compiler: C++11

To compile:
g++ -std=c++11 main.cpp -lpthread

To run:
./a FILE_PATH_OF_CARS OPTIONAL_NUMBER_OF_CONSECUTIVE

For example:
./a simple.txt
./a simple.txt 10

The program allows for N cars in one direction to go. Although the project specification states that 10 should be the default, this implementation's default is the standard (i.e. no consecutive cars) execution. This allows for easier testing to ensure that the base program is correct. The additional command line argument specifies the number of cars in one direction that can go at once.
