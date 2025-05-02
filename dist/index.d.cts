declare enum ResizeMode {
    Fixed = "fixed",
    Percent = "percent"
}
interface ResizeOptionsFixed {
    width: number;
    height: number;
    keepAspect: boolean;
}
interface ResizeOptionsPercent {
    percentValue: number;
}
declare enum AvifPixelFormat {
    YUV444 = 0,
    YUV422 = 1,
    YUV420 = 2,
    YUV400 = 3
}
declare enum CodecChoice {
    AUTO = 0,
    AOM = 1,
    SVT = 2
}
declare enum Tune {
    TUNE_DEFAULT = 0,
    TUNE_PSNR = 1,
    TUNE_SSIM = 2
}
declare enum ErrorCode {
    OK = 0,
    INVALID_DIMENSIONS = 100,
    IMAGE_LOAD_FAILED = 101,
    ENCODER_CREATION_FAILED = 102,
    CONVERSION_FAILED = 103,
    ENCODING_FAILED = 104,
    INVALID_ARGUMENT = 105,
    OUT_OF_MEMORY = 106,
    INVALID_QUANTIZER_VALUES = 107,
    UNKNOWN_ERROR = 108,
    AVIF_ERROR_START = 200
}
interface ConverterError {
    /** Error code */
    code: ErrorCode;
    /** Error message */
    message: string;
    /** Stack trace (if available) */
    stackTrace: string;
}
interface EncodeConfig {
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
interface ConvertImageParams {
    inputData: Uint8Array;
    width: number;
    height: number;
    config: EncodeConfig;
}
/**
 * Represents the raw image data buffer
 */
interface ImageBuffer {
    /** Returns a typed array view of the image data */
    getData(): Uint8Array;
    /** Returns the size of the buffer */
    getSize(): number;
}
/**
 * Smart pointer wrapper for ImageBuffer (std::shared_ptr<ImageBuffer>)
 */
interface SharedImageBuffer extends ImageBuffer {
    /** Frees the WASM-side memory, decrementing ref count */
    delete(): void;
}
/**
 * Result type that contains either a successful ImageBuffer or an Error
 */
interface ConversionResult {
    success: boolean;
    image?: ImageBuffer;
    error?: ConverterError;
}
interface ConvAvifModule {
    EncodeConfig: {
        new (): EncodeConfig;
    };
    CodecChoice: typeof CodecChoice;
    AvifPixelFormat: typeof AvifPixelFormat;
    Tune: typeof Tune;
    ErrorCode: typeof ErrorCode;
    convertImage(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): ConversionResult;
    convertImageDirect(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): {
        success: boolean;
        data?: Uint8Array;
        size?: number;
        error?: ConverterError;
    };
}

declare global {
    interface Window {
        importScripts?: (path: string) => void;
    }
}
/**
 * Initialize and return the WASM module instance
 */
declare function initWasm(): Promise<ConvAvifModule>;
/**
 * Compute target dimensions based on resize mode and options
 */
declare function computeDimensions(origWidth: number, origHeight: number, mode: ResizeMode, fixedOpts?: ResizeOptionsFixed, percentOpts?: ResizeOptionsPercent): {
    width: number;
    height: number;
};
/**
 * Convert raw image data via WASM and return AVIF bytes or throw an error
 */
declare function convertImage(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): Promise<Uint8Array>;
/**
 * High-level helper: convert and produce a Blob for download
 *
 * @throws Error with additional properties `code` and `stackTrace` when conversion fails
 */
declare function convertToBlob(params: ConvertImageParams): Promise<{
    data: Uint8Array;
    blob: Blob;
}>;

export { AvifPixelFormat, CodecChoice, type ConvAvifModule, type ConversionResult, type ConvertImageParams, type ConverterError, type EncodeConfig, ErrorCode, type ImageBuffer, ResizeMode, type ResizeOptionsFixed, type ResizeOptionsPercent, type SharedImageBuffer, Tune, computeDimensions, convertImage, convertToBlob, initWasm };
