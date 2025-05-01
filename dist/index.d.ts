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
/** Return type of Module.convertImage (std::shared_ptr<ImageBuffer>) */
type ConvertImageResult = SharedImageBuffer;
interface ConvAvifModule {
    convertImage(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): ConvertImageResult;
    EncodeConfig: {
        new (): EncodeConfig;
    };
    CodecChoice: typeof CodecChoice;
    AvifPixelFormat: typeof AvifPixelFormat;
    Tune: typeof Tune;
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
 * Convert raw image data via WASM and return AVIF bytes
 */
declare function convertImage(inputData: Uint8Array, width: number, height: number, config: EncodeConfig): Promise<Uint8Array>;
/**
 * High-level helper: convert and produce a Blob for download
 */
declare function convertToBlob(params: ConvertImageParams): Promise<{
    data: Uint8Array;
    blob: Blob;
}>;

export { AvifPixelFormat, CodecChoice, type ConvAvifModule, type ConvertImageParams, type ConvertImageResult, type EncodeConfig, type ImageBuffer, ResizeMode, type ResizeOptionsFixed, type ResizeOptionsPercent, type SharedImageBuffer, Tune, computeDimensions, convertImage, convertToBlob, initWasm };
