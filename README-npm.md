# ConvAvif NPM Package

A WebAssembly-based image converter for the AVIF format, enabling fast encoding and decoding in browser or Node.js environments.

## Installation

```bash
npm install convavif
```

## Usage

### Basic Image Conversion

```javascript
import { initWasm, convertImage } from 'convavif';

// Initialize the WASM module
const main = async () => {
  // Initialize the module (loads WASM)
  const mod = await initWasm();

  // RGB input data (your image bytes)
  const imageData = new Uint8Array([/* RGB pixel data */]);
  const width = 800;
  const height = 600;

  // Create encoding configuration
  const config = new mod.EncodeConfig();
  config.quality = 80;
  config.pixelFormat = mod.AvifPixelFormat.YUV420;
  
  // Convert image to AVIF
  const avifBytes = await convertImage(imageData, width, height, config);
  
  // Do something with the AVIF bytes...
  // e.g., create a Blob: new Blob([avifBytes], { type: 'image/avif' })
};

main();
```

### Browser Example with File Input

```javascript
import { initWasm, convertToBlob, computeDimensions, ResizeMode } from 'convavif';

document.getElementById('convertBtn').addEventListener('click', async () => {
  const fileInput = document.getElementById('fileInput');
  if (!fileInput.files.length) return;
  
  const file = fileInput.files[0];
  const data = new Uint8Array(await file.arrayBuffer());
  
  // Create an image to get dimensions
  const img = new Image();
  img.src = URL.createObjectURL(file);
  await img.decode();
  
  // Calculate dimensions (optional resize)
  const { width, height } = computeDimensions(
    img.naturalWidth, img.naturalHeight,
    ResizeMode.Fixed, 
    { width: img.naturalWidth, height: img.naturalHeight, keepAspect: true }
  );
  
  // Initialize WASM module
  const mod = await initWasm();
  
  // Configure encoding
  const config = new mod.EncodeConfig();
  config.quality = 80;
  
  // Convert and get blob
  const { blob } = await convertToBlob({ inputData: data, width, height, config });
  
  // Display or download the result
  document.getElementById('output').src = URL.createObjectURL(blob);
});
```

## API Reference

### Main Functions

- `initWasm()`: Initializes the WASM module
- `convertImage(inputData, width, height, config)`: Converts raw image data to AVIF bytes
- `convertToBlob(params)`: High-level function that returns an AVIF blob
- `computeDimensions(origWidth, origHeight, mode, fixedOpts?, percentOpts?)`: Utility for calculating resize dimensions

### Types and Enums

- `ResizeMode`: Resize mode options (`Fixed`, `Percent`)
- `AvifPixelFormat`: AVIF pixel formats (`YUV444`, `YUV422`, `YUV420`, `YUV400`)
- `CodecChoice`: AV1 codec options (`AUTO`, `AOM`, `SVT`)
- `Tune`: Encoder tuning parameters (`TUNE_DEFAULT`, `TUNE_PSNR`, `TUNE_SSIM`)

## Browser Compatibility

ConvAvif works in all modern browsers that support WebAssembly, including:

- Chrome/Edge 79+
- Firefox 72+
- Safari 15.2+
- Opera 66+

## License

MIT License
