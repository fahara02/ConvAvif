// TypeScript definitions for AVIF converter config and UI types

// Resize modes in the UI
export enum ResizeMode {
  Fixed = 'fixed',
  Percent = 'percent',
}

// Fixed resize options
export interface ResizeOptionsFixed {
  width: number;
  height: number;
  keepAspect: boolean;
}

// Percentage-based resize options
export interface ResizeOptionsPercent {
  percentValue: number;
}

// AVIF pixel formats (match avifPixelFormat in C++)
export enum AvifPixelFormat {
  YUV444 = 0,
  YUV422 = 1,
  YUV420 = 2,
  YUV400 = 3,
}

// Codec choices (match EncodeConfig::CodecChoice)
export enum CodecChoice {
  AUTO = 0,
  AOM = 1,
  SVT = 2,
}

// Tuning parameters (match EncodeConfig::Tune)
export enum Tune {
  TUNE_DEFAULT = 0,
  TUNE_PSNR = 1,
  TUNE_SSIM = 2,
}

// Error codes (match ConverterError enum in C++)
export enum ErrorCode {
  // Success
  OK = 0,

  // Application-specific errors
  INVALID_DIMENSIONS = 100,
  IMAGE_LOAD_FAILED,
  ENCODER_CREATION_FAILED,
  CONVERSION_FAILED,
  ENCODING_FAILED,
  INVALID_ARGUMENT,
  OUT_OF_MEMORY,
  INVALID_QUANTIZER_VALUES,
  UNKNOWN_ERROR,

  // AVIF errors start at 200 (offset to avoid overlap)
  AVIF_ERROR_START = 200
}

// Error object returned from C++ (matches Error class in C++)
export interface ConverterError {
  /** Error code */
  code: ErrorCode;
  /** Error message */
  message: string;
  /** Stack trace (if available) */
  stackTrace: string;
}

// Encoding configuration (match EncodeConfig struct in C++)
export interface EncodeConfig {
  quality: number;
  qualityAlpha: number;
  speed: number;
  sharpness: number;
  pixelFormat: AvifPixelFormat;
  codecChoice: CodecChoice;
  minQuantizer: number;
  maxQuantizer: number;
  tileRowsLog2: number;
  tileColsLog2: number;
  tune: Tune;
}

// Parameters for the convertImage call
export interface ConvertImageParams {
  inputData: Uint8Array;
  width: number;
  height: number;
  config: EncodeConfig;
}

/**
 * Represents the raw image data buffer
 */
export interface ImageBuffer {
  /** Returns a typed array view of the image data */
  getData(): Uint8Array;
  /** Returns the size of the buffer */
  getSize(): number;
}

/**
 * Smart pointer wrapper for ImageBuffer (std::shared_ptr<ImageBuffer>)
 */
export interface SharedImageBuffer extends ImageBuffer {
  /** Frees the WASM-side memory, decrementing ref count */
  delete(): void;
}

/**
 * Result type that contains either a successful ImageBuffer or an Error
 */
export interface ConversionResult {
  // Whether the conversion was successful
  success: boolean;
  // The image if conversion was successful, undefined otherwise
  image?: ImageBuffer;
  // The error if conversion failed, undefined otherwise
  error?: ConverterError;
}

export interface ConvAvifModule {
  EncodeConfig: { new(): EncodeConfig };
  CodecChoice: typeof CodecChoice;
  AvifPixelFormat: typeof AvifPixelFormat;
  Tune: typeof Tune;
  ErrorCode: typeof ErrorCode;
  
  // Original method (now used as a fallback)
  convertImage(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): ConversionResult;
  
  // New direct conversion method that provides reliable data transfer
  convertImageDirect(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): {
    success: boolean;
    data?: Uint8Array;
    size?: number;
    error?: ConverterError;
  };
}
