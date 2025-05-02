// Wrapper for ConvAvif WASM module and utility functions
import {
    ConvAvifModule,
    ConvertImageParams,
    ConversionResult,
    EncodeConfig,
    ResizeMode,
    ResizeOptionsFixed,
    ResizeOptionsPercent,
    AvifPixelFormat,
    CodecChoice,
    Tune,
    ErrorCode,
    ConverterError
} from './types.js';

// Allow for both build-time and runtime import resolution
// In build/dev, import from the build directory
// In the published package, these paths are transformed to './convavif.mjs'
declare const require: any;

// This is resolved at runtime, not build time
declare global {
  interface Window {
    importScripts?: (path: string) => void;
  }
}

/**
 * Load the WASM module, handling different environments
 */
const loadModuleFactory = async (): Promise<any> => {
  // Define paths based on environment
  const paths = [
    // Path 1: When importing as a package - use wasm.mjs export
    './wasm.mjs',
    // Path 2: Direct path to the dist folder
    './imageconverter.js',
    // Path 3: Development path (relative to src folder)
    '../dist/imageconverter.js',
    // Path 4: Browser relative path
    '/dist/imageconverter.js'
  ];

  let lastError: Error | null = null;

  // Try each path in sequence
  for (const path of paths) {
    try {
      if (typeof window !== 'undefined') {
        // Browser environment
        const dynamicImport = new Function('path', 'return import(path)');
        const mod = await dynamicImport(path);
        return mod.default || mod;
      } else {
        // Node environment - dynamic import with relative path
        // @ts-ignore - Dynamic import in Node
        const mod = await import(path);
        return mod.default || mod;
      }
    } catch (e) {
      lastError = e as Error;
      console.warn(`Failed to load WASM from path: ${path}`);
      // Continue to next path
    }
  }

  // All paths failed
  console.error('All module loading attempts failed');
  throw new Error(`Failed to load WASM module: ${lastError?.message || 'Unknown error'}`);
};
// Types are already imported at the top of the file



let wasmModule: ConvAvifModule;

/**
 * Initialize and return the WASM module instance
 */
export async function initWasm(): Promise<ConvAvifModule> {
  if (!wasmModule) {
    const moduleFactory = await loadModuleFactory();
    const instance: any = await moduleFactory();
    wasmModule = instance as ConvAvifModule;
  }
  return wasmModule;
}

/**
 * Compute target dimensions based on resize mode and options
 */
export function computeDimensions(
  origWidth: number,
  origHeight: number,
  mode: ResizeMode,
  fixedOpts?: ResizeOptionsFixed,
  percentOpts?: ResizeOptionsPercent
): { width: number; height: number } {
  let width: number;
  let height: number;
  if (mode === ResizeMode.Percent && percentOpts) {
    width = Math.round(origWidth * percentOpts.percentValue / 100);
    height = Math.round(origHeight * percentOpts.percentValue / 100);
  } else if (mode === ResizeMode.Fixed && fixedOpts) {
    width = fixedOpts.width;
    height = fixedOpts.keepAspect
      ? Math.round(fixedOpts.width * origHeight / origWidth)
      : fixedOpts.height;
  } else {
    width = origWidth;
    height = origHeight;
  }
  return { width, height };
}

/**
 * Convert raw image data via WASM and return AVIF bytes or throw an error
 */
export async function convertImage(
  inputData: Uint8Array,
  width: number,
  height: number,
  config: EncodeConfig
): Promise<Uint8Array> {
  const mod = await initWasm();
  // Use the direct conversion method for reliable data transfer
  const result = mod.convertImageDirect(inputData, width, height, config);
  
  // Check if there was an error
  if (!result.success && result.error) {
    const error = result.error;
    const errorObj = new Error(error.message);
    // Add custom properties
    (errorObj as any).code = error.code;
    (errorObj as any).stackTrace = error.stackTrace;
    throw errorObj;
  }
  
  // If successful, get the image data directly
  if (result.success && result.data) {
    // data is already a Uint8Array, just create a copy to ensure it's detached
    const copy = new Uint8Array(result.data);
    return copy;
  }
  
  throw new Error('Conversion successful but no image data returned');
}

/**
 * High-level helper: convert and produce a Blob for download
 * 
 * @throws Error with additional properties `code` and `stackTrace` when conversion fails
 */
export async function convertToBlob(
  params: ConvertImageParams
): Promise<{ data: Uint8Array; blob: Blob }> {
  try {
    const data = await convertImage(params.inputData, params.width, params.height, params.config);
    const blob = new Blob([data], { type: 'image/avif' });
    return { data, blob };
  } catch (error) {
    // Re-throw the error from convertImage
    throw error;
  }
}
