# ConvAvif

A WebAssembly-based image converter for the AVIF format, enabling fast encoding and decoding in browser or Node.js environments.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building](#building)
- [Usage](#usage)
  - [In Browser](#in-browser)
  - [In Node.js](#in-nodejs)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Features

- Encode JPEG/PNG images to AVIF
- Decode AVIF to RGBA pixel data
- WebAssembly (WASM) module with JavaScript bindings
- Lightweight, high-performance, and portable

## Prerequisites

- CMake ≥ 3.13
- [Emscripten SDK](https://emscripten.org/)
- [Node.js](https://nodejs.org/) (optional, for CLI/testing)

## Installation

```bash
git clone https://github.com/fahara02/ConvAvif.git
cd ConvAvif
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
emmake make
```

## Building

Activate the Emscripten environment and generate build files:

> **Windows (Binaryen) Note**
> If you're on Windows, ensure Binaryen is installed by following [COMPILING_WIN32 instructions](https://github.com/brakmic/bazaar/blob/master/webassembly/COMPILING_WIN32.md).
> After installing, run:
> ```bash
> emcc --generate-config
> ```
> This configures the path to `wasm-opt.exe`. Otherwise, `emmake` may fail with a `binaryen executable not found` error.

```bash
# Initialize and activate emsdk (adjust path as needed)
source /path/to/emsdk/emsdk_env.sh

# Configure build with CMake
emcmake cmake -S . -B build

# Compile
cmake --build build
```

Upon success, the `build/` directory will contain:

- `imageconverter.js`
- `imageconverter.wasm`

## Usage

### In Browser

Include the generated script in your HTML page:

```html
<script src="build/imageconverter.js"></script>
<script>
  Module.onRuntimeInitialized = () => {
    // Example: call a function to encode raw pixel data
    // Module._encodeToAvif(ptr, width, height, quality);
  };
</script>
```

### In Node.js

```js
const createModule = require('./build/imageconverter.js');

createModule().then(Module => {
  // Example: use Module._encodeToAvif(...)
});
```

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a feature branch: `git checkout -b feature/awesome`.
3. Commit your changes and push: `git push origin feature/awesome`.
4. Open a Pull Request against `main`.

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgements

- [libavif](https://github.com/AOMediaCodec/libavif)
- [Emscripten](https://emscripten.org/)
- Inspired by community efforts in high-efficiency image formats.
