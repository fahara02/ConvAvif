@echo off
echo Building WASM module...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Clean CMake cache
if exist build\CMakeCache.txt del /f build\CMakeCache.txt
if exist build\CMakeFiles rmdir /s /q build\CMakeFiles

cd build

REM Configure with CMake
echo Running CMake configuration...
call emcmake cmake -DCMAKE_BUILD_TYPE=Release ..

REM Build
echo Building with Emscripten...
call emmake make

REM Check if build succeeded
if %ERRORLEVEL% NEQ 0 (
  echo Build failed with error code %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

echo WASM build completed successfully.
cd ..
