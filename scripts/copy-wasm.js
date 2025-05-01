import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Define paths
const buildDir = path.join(__dirname, '..', 'build');
const distDir = path.join(__dirname, '..', 'dist');

// Ensure dist directory exists
if (!fs.existsSync(distDir)) {
  fs.mkdirSync(distDir, { recursive: true });
}

// Files to copy from build to dist
const filesToCopy = [
  'imageconverter.js',
  'imageconverter.wasm'
];

console.log('Copying WASM files from build to dist...');

// Copy each file
filesToCopy.forEach(file => {
  const sourcePath = path.join(buildDir, file);
  const targetPath = path.join(distDir, file);
  
  if (fs.existsSync(sourcePath)) {
    fs.copyFileSync(sourcePath, targetPath);
    console.log(`✓ Copied: ${sourcePath} -> ${targetPath}`);
  } else {
    console.error(`✗ Error: Source file not found: ${sourcePath}`);
  }
});

// Also create a convavif.mjs symlink to imageconverter.js for module imports
const sourceJsPath = path.join(distDir, 'imageconverter.js');
const targetMjsPath = path.join(distDir, 'convavif.mjs');

if (fs.existsSync(sourceJsPath)) {
  try {
    fs.copyFileSync(sourceJsPath, targetMjsPath);
    console.log(`✓ Created: ${targetMjsPath}`);
  } catch (err) {
    console.error(`✗ Error creating mjs file: ${err.message}`);
  }
} else {
  console.error(`✗ Error: Source JS file not found: ${sourceJsPath}`);
}

// Create a module wrapper file to support the package.json exports field
const wrapperContent = `// Auto-generated WASM wrapper for package exports
export { default } from './imageconverter.js';
`;

try {
  fs.writeFileSync(path.join(distDir, 'wasm.mjs'), wrapperContent);
  console.log(`✓ Created wasm module wrapper: ${path.join(distDir, 'wasm.mjs')}`);
} catch (err) {
  console.error(`✗ Error creating wasm wrapper file: ${err.message}`);
}

console.log('WASM files copy completed');
