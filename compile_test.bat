cmake -S . -B .\build\
cmake --build .\build\
cd .\build\
ctest -C Debug -V
cd ..
