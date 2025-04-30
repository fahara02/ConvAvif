@echo off
setlocal

mkdir build
cd build

emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
emmake make

emcc -O3 --bind -o ../dist/imageconverter.js \
    imageconverter.cpp \
    -sEXPORTED_FUNCTIONS=_convert_image,_free_buffer \
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,allocate,intArrayFromString \
    -sALLOW_MEMORY_GROWTH=1 \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -lavif

cd ..