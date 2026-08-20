set shell := ["bash", "-cu"]

build:
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build build -j
    cp build/compile_commands.json ./compile_commands.json

run: build
    ./build/rheo

clean:
    rm -rf build
    rm -f compile_commands.json

rebuild: clean build
