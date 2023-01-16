mkdir build
cd build
rm ./connectFour
cmake -S ..
make -S
cd ..
./build/connectFour