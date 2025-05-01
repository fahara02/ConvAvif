// Wrapper for ConvAvif WASM module and utility functions
import {
    ConvAvifModule,
    ConvertImageParams,
    ConvertImageResult,
    EncodeConfig,
    ResizeMode,
    ResizeOptionsFixed,
    ResizeOptionsPercent,
    AvifPixelFormat,
    CodecChoice,
    Tune
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
 * Convert raw image data via WASM and return AVIF bytes
 */
export async function convertImage(
  inputData: Uint8Array,
  width: number,
  height: number,
  config: EncodeConfig
): Promise<Uint8Array> {
  const mod = await initWasm();
  const result: ConvertImageResult = mod.convertImage(inputData, width, height, config);
  const view = result.getData();
  const copy = new Uint8Array(view);
  result.delete();
  return copy;
}

/**
 * High-level helper: convert and produce a Blob for download
 */
export async function convertToBlob(
  params: ConvertImageParams
): Promise<{ data: Uint8Array; blob: Blob }> {
  const data = await convertImage(params.inputData, params.width, params.height, params.config);
  const blob = new Blob([data], { type: 'image/avif' });
  return { data, blob };
}
