import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Define paths
const PROJECT_ROOT = path.join(__dirname, '..');
const BUILD_DIR = path.join(PROJECT_ROOT, 'build');

// Clean the build directory to avoid generator conflicts
const cleanBuild = () => {
  console.log('Cleaning CMake cache...');
  // Delete CMakeCache.txt and CMakeFiles to avoid generator conflicts
  if (fs.existsSync(path.join(BUILD_DIR, 'CMakeCache.txt'))) {
    fs.unlinkSync(path.join(BUILD_DIR, 'CMakeCache.txt'));
  }
  
  try {
    if (fs.existsSync(path.join(BUILD_DIR, 'CMakeFiles'))) {
      fs.rmSync(path.join(BUILD_DIR, 'CMakeFiles'), { recursive: true, force: true });
    }
  } catch (e) {
    console.warn('Warning: Could not remove CMakeFiles directory. Please delete manually if build fails.');
  }
};

// Create build directory if it doesn't exist
if (!fs.existsSync(BUILD_DIR)) {
  console.log('Creating build directory...');
  fs.mkdirSync(BUILD_DIR, { recursive: true });
} else {
  cleanBuild();
}

try {
  // Change to build directory
  console.log('Changing to build directory...');
  process.chdir(BUILD_DIR);
  
  // Run CMake configure with explicit Ninja generator
  console.log('Running emcmake cmake...');
  execSync('emcmake cmake -DCMAKE_BUILD_TYPE=Release .. -G "Ninja"', { 
    stdio: 'inherit',
    shell: true 
  });
  
  // Run make
  console.log('Running emmake make...');
  execSync('emmake ninja', { 
    stdio: 'inherit',
    shell: true 
  });
  
  console.log('WASM build completed successfully.');
} catch (error) {
  console.error('Error building WASM:', error.message);
  
  // Second attempt with MinGW if Ninja fails
  try {
    console.log('\nAttempting alternate build with MinGW...');
    cleanBuild();
    
    execSync('emcmake cmake -DCMAKE_BUILD_TYPE=Release .. -G "MinGW Makefiles"', { 
      stdio: 'inherit',
      shell: true 
    });
    
    execSync('emmake make', { 
      stdio: 'inherit',
      shell: true 
    });
    
    console.log('WASM build completed successfully with MinGW.');
  } catch (secondError) {
    console.error('Error on second build attempt:', secondError.message);
    process.exit(1);
  }
}
