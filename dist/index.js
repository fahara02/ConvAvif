// src/types.ts
var ResizeMode = /* @__PURE__ */ ((ResizeMode2) => {
  ResizeMode2["Fixed"] = "fixed";
  ResizeMode2["Percent"] = "percent";
  return ResizeMode2;
})(ResizeMode || {});
var AvifPixelFormat = /* @__PURE__ */ ((AvifPixelFormat3) => {
  AvifPixelFormat3[AvifPixelFormat3["YUV444"] = 0] = "YUV444";
  AvifPixelFormat3[AvifPixelFormat3["YUV422"] = 1] = "YUV422";
  AvifPixelFormat3[AvifPixelFormat3["YUV420"] = 2] = "YUV420";
  AvifPixelFormat3[AvifPixelFormat3["YUV400"] = 3] = "YUV400";
  return AvifPixelFormat3;
})(AvifPixelFormat || {});
var CodecChoice = /* @__PURE__ */ ((CodecChoice3) => {
  CodecChoice3[CodecChoice3["AUTO"] = 0] = "AUTO";
  CodecChoice3[CodecChoice3["AOM"] = 1] = "AOM";
  CodecChoice3[CodecChoice3["SVT"] = 2] = "SVT";
  return CodecChoice3;
})(CodecChoice || {});
var Tune = /* @__PURE__ */ ((Tune3) => {
  Tune3[Tune3["TUNE_DEFAULT"] = 0] = "TUNE_DEFAULT";
  Tune3[Tune3["TUNE_PSNR"] = 1] = "TUNE_PSNR";
  Tune3[Tune3["TUNE_SSIM"] = 2] = "TUNE_SSIM";
  return Tune3;
})(Tune || {});
var ErrorCode = /* @__PURE__ */ ((ErrorCode3) => {
  ErrorCode3[ErrorCode3["OK"] = 0] = "OK";
  ErrorCode3[ErrorCode3["INVALID_DIMENSIONS"] = 100] = "INVALID_DIMENSIONS";
  ErrorCode3[ErrorCode3["IMAGE_LOAD_FAILED"] = 101] = "IMAGE_LOAD_FAILED";
  ErrorCode3[ErrorCode3["ENCODER_CREATION_FAILED"] = 102] = "ENCODER_CREATION_FAILED";
  ErrorCode3[ErrorCode3["CONVERSION_FAILED"] = 103] = "CONVERSION_FAILED";
  ErrorCode3[ErrorCode3["ENCODING_FAILED"] = 104] = "ENCODING_FAILED";
  ErrorCode3[ErrorCode3["INVALID_ARGUMENT"] = 105] = "INVALID_ARGUMENT";
  ErrorCode3[ErrorCode3["OUT_OF_MEMORY"] = 106] = "OUT_OF_MEMORY";
  ErrorCode3[ErrorCode3["INVALID_QUANTIZER_VALUES"] = 107] = "INVALID_QUANTIZER_VALUES";
  ErrorCode3[ErrorCode3["UNKNOWN_ERROR"] = 108] = "UNKNOWN_ERROR";
  ErrorCode3[ErrorCode3["AVIF_ERROR_START"] = 200] = "AVIF_ERROR_START";
  return ErrorCode3;
})(ErrorCode || {});

// src/convAvif.ts
var loadModuleFactory = async () => {
  const paths = [
    // Path 1: When importing as a package - use wasm.mjs export
    "./wasm.mjs",
    // Path 2: Direct path to the dist folder
    "./imageconverter.js",
    // Path 3: Development path (relative to src folder)
    "../dist/imageconverter.js",
    // Path 4: Browser relative path
    "/dist/imageconverter.js"
  ];
  let lastError = null;
  for (const path of paths) {
    try {
      if (typeof window !== "undefined") {
        const dynamicImport = new Function("path", "return import(path)");
        const mod = await dynamicImport(path);
        return mod.default || mod;
      } else {
        const mod = await import(path);
        return mod.default || mod;
      }
    } catch (e) {
      lastError = e;
      console.warn(`Failed to load WASM from path: ${path}`);
    }
  }
  console.error("All module loading attempts failed");
  throw new Error(`Failed to load WASM module: ${lastError?.message || "Unknown error"}`);
};
var wasmModule;
async function initWasm() {
  if (!wasmModule) {
    const moduleFactory = await loadModuleFactory();
    const instance = await moduleFactory();
    wasmModule = instance;
  }
  return wasmModule;
}
function computeDimensions(origWidth, origHeight, mode, fixedOpts, percentOpts) {
  let width;
  let height;
  if (mode === "percent" /* Percent */ && percentOpts) {
    width = Math.round(origWidth * percentOpts.percentValue / 100);
    height = Math.round(origHeight * percentOpts.percentValue / 100);
  } else if (mode === "fixed" /* Fixed */ && fixedOpts) {
    width = fixedOpts.width;
    height = fixedOpts.keepAspect ? Math.round(fixedOpts.width * origHeight / origWidth) : fixedOpts.height;
  } else {
    width = origWidth;
    height = origHeight;
  }
  return { width, height };
}
async function convertImage(inputData, width, height, config) {
  const mod = await initWasm();
  const result = mod.convertImageDirect(inputData, width, height, config);
  if (!result.success && result.error) {
    const error = result.error;
    const errorObj = new Error(error.message);
    errorObj.code = error.code;
    errorObj.stackTrace = error.stackTrace;
    throw errorObj;
  }
  if (result.success && result.data) {
    const copy = new Uint8Array(result.data);
    return copy;
  }
  throw new Error("Conversion successful but no image data returned");
}
async function convertToBlob(params) {
  try {
    const data = await convertImage(params.inputData, params.width, params.height, params.config);
    const blob = new Blob([data], { type: "image/avif" });
    return { data, blob };
  } catch (error) {
    throw error;
  }
}
export {
  AvifPixelFormat,
  CodecChoice,
  ErrorCode,
  ResizeMode,
  Tune,
  computeDimensions,
  convertImage,
  convertToBlob,
  initWasm
};
