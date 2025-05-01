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

// src/convAvif.ts
var loadModuleFactory = async () => {
  const wasmPath = "./convavif.mjs";
  try {
    const dynamicImport = new Function("path", "return import(path)");
    const mod = await dynamicImport(wasmPath);
    return mod.default || mod;
  } catch (e) {
    const error = e;
    console.error("Error loading WASM module:", error);
    console.warn("Attempting development fallback path...");
    try {
      const mod = await import("../build/imageconverter.js");
      return mod.default || mod;
    } catch (fallbackError) {
      console.error("Fallback import also failed:", fallbackError);
      throw new Error(`Failed to load WASM module: ${error.message || "Unknown error"}`);
    }
  }
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
  const result = mod.convertImage(inputData, width, height, config);
  const view = result.getData();
  const copy = new Uint8Array(view);
  result.delete();
  return copy;
}
async function convertToBlob(params) {
  const data = await convertImage(params.inputData, params.width, params.height, params.config);
  const blob = new Blob([data], { type: "image/avif" });
  return { data, blob };
}
export {
  AvifPixelFormat,
  CodecChoice,
  ResizeMode,
  Tune,
  computeDimensions,
  convertImage,
  convertToBlob,
  initWasm
};
