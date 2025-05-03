"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// src/index.ts
var index_exports = {};
__export(index_exports, {
  AvifPixelFormat: () => AvifPixelFormat,
  CodecChoice: () => CodecChoice,
  ErrorCode: () => ErrorCode,
  ResizeMode: () => ResizeMode,
  SpeedPreset: () => SpeedPreset,
  Tune: () => Tune,
  computeDimensions: () => computeDimensions,
  convertImage: () => convertImage,
  convertToBlob: () => convertToBlob,
  initWasm: () => initWasm
});
module.exports = __toCommonJS(index_exports);

// src/types.ts
var ResizeMode = /* @__PURE__ */ ((ResizeMode2) => {
  ResizeMode2["Fixed"] = "fixed";
  ResizeMode2["Percent"] = "percent";
  return ResizeMode2;
})(ResizeMode || {});
var AvifPixelFormat = /* @__PURE__ */ ((AvifPixelFormat2) => {
  AvifPixelFormat2[AvifPixelFormat2["YUV444"] = 0] = "YUV444";
  AvifPixelFormat2[AvifPixelFormat2["YUV422"] = 1] = "YUV422";
  AvifPixelFormat2[AvifPixelFormat2["YUV420"] = 2] = "YUV420";
  AvifPixelFormat2[AvifPixelFormat2["YUV400"] = 3] = "YUV400";
  return AvifPixelFormat2;
})(AvifPixelFormat || {});
var CodecChoice = /* @__PURE__ */ ((CodecChoice2) => {
  CodecChoice2[CodecChoice2["AUTO"] = 0] = "AUTO";
  CodecChoice2[CodecChoice2["AOM"] = 1] = "AOM";
  CodecChoice2[CodecChoice2["SVT"] = 2] = "SVT";
  return CodecChoice2;
})(CodecChoice || {});
var Tune = /* @__PURE__ */ ((Tune2) => {
  Tune2[Tune2["TUNE_DEFAULT"] = 0] = "TUNE_DEFAULT";
  Tune2[Tune2["TUNE_PSNR"] = 1] = "TUNE_PSNR";
  Tune2[Tune2["TUNE_SSIM"] = 2] = "TUNE_SSIM";
  return Tune2;
})(Tune || {});
var SpeedPreset = /* @__PURE__ */ ((SpeedPreset2) => {
  SpeedPreset2["MemoryHungry"] = "MemoryHungry";
  SpeedPreset2["Good"] = "Good";
  SpeedPreset2["RealTime"] = "RealTime";
  return SpeedPreset2;
})(SpeedPreset || {});
var ErrorCode = /* @__PURE__ */ ((ErrorCode2) => {
  ErrorCode2[ErrorCode2["OK"] = 0] = "OK";
  ErrorCode2[ErrorCode2["INVALID_DIMENSIONS"] = 100] = "INVALID_DIMENSIONS";
  ErrorCode2[ErrorCode2["IMAGE_LOAD_FAILED"] = 101] = "IMAGE_LOAD_FAILED";
  ErrorCode2[ErrorCode2["ENCODER_CREATION_FAILED"] = 102] = "ENCODER_CREATION_FAILED";
  ErrorCode2[ErrorCode2["CONVERSION_FAILED"] = 103] = "CONVERSION_FAILED";
  ErrorCode2[ErrorCode2["ENCODING_FAILED"] = 104] = "ENCODING_FAILED";
  ErrorCode2[ErrorCode2["INVALID_ARGUMENT"] = 105] = "INVALID_ARGUMENT";
  ErrorCode2[ErrorCode2["OUT_OF_MEMORY"] = 106] = "OUT_OF_MEMORY";
  ErrorCode2[ErrorCode2["INVALID_QUANTIZER_VALUES"] = 107] = "INVALID_QUANTIZER_VALUES";
  ErrorCode2[ErrorCode2["UNKNOWN_ERROR"] = 108] = "UNKNOWN_ERROR";
  ErrorCode2[ErrorCode2["AVIF_ERROR_START"] = 200] = "AVIF_ERROR_START";
  return ErrorCode2;
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
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  AvifPixelFormat,
  CodecChoice,
  ErrorCode,
  ResizeMode,
  SpeedPreset,
  Tune,
  computeDimensions,
  convertImage,
  convertToBlob,
  initWasm
});
