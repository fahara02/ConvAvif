import { defineConfig } from 'tsup';

export default defineConfig({
  format: ['cjs', 'esm'],
  entry: ['./src/index.ts'],
  dts: true,
  shims: true,
  skipNodeModulesBundle: true,
  clean: true,
  noExternal: ['./src/**'],
  external: ['../build/imageconverter.js'],
  esbuildOptions(options) {
    // Prevent bundling the WASM module
    options.external = ['../build/imageconverter.js'];
  }
});