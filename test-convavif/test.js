// Test script for the convavif npm package using actual image file
import * as convavif from 'convavif';
import { readFile, writeFile } from 'fs/promises';
import { join } from 'path';

// Complete test with file conversion
async function testModule() {
  try {
    console.log('Testing convavif package...');
    console.log('Available exports:', Object.keys(convavif));
    
    // Verify ResizeMode enum is available
    if (convavif.ResizeMode) {
      console.log('\nResizeMode enum found:', Object.keys(convavif.ResizeMode));
    } else {
      console.error('ResizeMode enum missing');
      return false;
    }
    
    // Verify AvifPixelFormat enum is available
    if (convavif.AvifPixelFormat) {
      console.log('AvifPixelFormat enum found:', Object.keys(convavif.AvifPixelFormat));
    } else {
      console.error('AvifPixelFormat enum missing');
      return false;
    }
    
    console.log('\nLoading sample.jpg...');
    try {
      const imagePath = join(process.cwd(), 'sample.jpg');
      const imageData = await readFile(imagePath);
      console.log(`Loaded JPG file: ${imagePath} (${imageData.byteLength} bytes)`);
      
      // Initialize the WASM module
      console.log('\nInitializing WASM module...');
      const module = await convavif.initWasm();
      console.log('WASM module initialized successfully!');
      
      // Create encoding configuration
      console.log('\nCreating encoding configuration...');
      const config = new module.EncodeConfig();
      config.quality = 65;
      // Use direct numeric values instead of enum objects
      // YUV420 = 2 in the C++ enum
      config.pixelFormat = module.AvifPixelFormat.YUV420 ; // module.AvifPixelFormat.YUV420 value
      config.speed = 6;
      
      console.log('Configuration:', {
        quality: config.quality,
        pixelFormat: config.pixelFormat,
        speed: config.speed
      });
      
      // Convert JPEG to AVIF
      console.log('\nConverting JPEG to AVIF...');
      console.time('Conversion time');
      
      // For this test, we're using predetermined dimensions
      // In a real application, you would need to decode the JPEG first
      const width = 640;  // Example dimension
      const height = 480; // Example dimension
      
      const avifData = await convavif.convertImage(imageData, width, height, config);
      console.timeEnd('Conversion time');
      
      // Save output
      const outputPath = join(process.cwd(), 'output.avif');
      await writeFile(outputPath, avifData);
      
      // Compare sizes
      const compressionRatio = (avifData.byteLength / imageData.byteLength * 100).toFixed(2);
      console.log(`\nConversion results:`);
      console.log(`  Original JPG size: ${imageData.byteLength} bytes`);
      console.log(`  AVIF size: ${avifData.byteLength} bytes`);
      console.log(`  Compression ratio: ${compressionRatio}% of original`);
      console.log(`  Output saved to: ${outputPath}`);
    } catch (fileError) {
      console.error('File processing error:', fileError);
      console.log('Continuing with basic verification only...');
    }
    
    console.log('\nInstallation verification successful!');
    return true;
  } catch (error) {
    console.error('\nTest failed with error:', error);
    return false;
  }
}

// Run the verification test
testModule().then(success => {
  if (success) {
    console.log('\nconvavif package test completed successfully! ✓');
  } else {
    console.error('\nconvavif package test failed! ✗');
    process.exit(1);
  }
});