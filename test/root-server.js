import express from 'express';
import path from 'path';
import { fileURLToPath } from 'url';

const app = express();
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Required security headers
app.use((req, res, next) => {
  res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
  res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
  next();
});

// Define paths relative to project root
const PROJECT_ROOT = path.join(__dirname, '..');

// Serve static files from various directories
app.use('/build', express.static(path.join(PROJECT_ROOT, 'build')));
app.use('/asset', express.static(path.join(PROJECT_ROOT, 'asset')));
app.use('/dist', express.static(path.join(PROJECT_ROOT, 'dist')));

// Serve test/basic directory files
app.use(express.static(path.join(PROJECT_ROOT, 'test', 'basic')));

// Default route serves the test page
app.get('/', (req, res) => {
  res.sendFile(path.join(PROJECT_ROOT, 'test', 'basic', 'index.html'));
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  console.log('CORS headers enabled for SharedArrayBuffer support');
});
