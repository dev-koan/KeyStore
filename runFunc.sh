g++ -std=c++20 -Iinclude src/*.cpp test.cpp -o runFunc.exe

./runFunc.exe crash1
./runFunc.exe verify1

./runFunc.exe crash2
./runFunc.exe verify2

./runFunc.exe delete