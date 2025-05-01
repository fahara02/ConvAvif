// Wrapper for ConvAvif WASM module and utility functions

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
 * We load the WASM module at runtime, as it's shipped as a standalone file
 */
const loadModuleFactory = async (): Promise<any> => {
  // Path to WASM module (runtime path, not build path)
  const wasmPath = './convavif.mjs'; // Same path for browser and Node.js

  try {
    // Use dynamic import for both browser and Node.js ES modules
    // This works in both environments for ESM
    const dynamicImport = new Function('path', 'return import(path)');
    const mod = await dynamicImport(wasmPath);
    return mod.default || mod;
  } catch (e: unknown) {
    const error = e as Error;
    console.error('Error loading WASM module:', error);
    console.warn('Attempting development fallback path...');
    
    try {
      // Try direct import with relative path for development
      // This is useful during development when linking locally
      // @ts-ignore - Allow dynamic import during development
      const mod = await import('../build/imageconverter.js');
      return mod.default || mod;
    } catch (fallbackError) {
      console.error('Fallback import also failed:', fallbackError);
      throw new Error(`Failed to load WASM module: ${error.message || 'Unknown error'}`);
    }
  }
};
import {
    ConvAvifModule,
    ConvertImageParams,
    ConvertImageResult,
    EncodeConfig,
    ResizeMode,
    ResizeOptionsFixed,
    ResizeOptionsPercent
} from './types';



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
