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
const PROJECT_ROOT = path.join(__dirname, '../docs/');

// Serve static files from various directories
app.use('/api', express.static(path.join(PROJECT_ROOT, 'api')));
app.use('/asset', express.static(path.join(PROJECT_ROOT, 'asset')));




// Default route serves the test page
app.get('/', (req, res) => {
  res.sendFile(path.join(PROJECT_ROOT, 'index.html'));
});

// Default route serves the test page
app.get('/api', (req, res) => {
  res.sendFile(path.join(PROJECT_ROOT, 'api','index.html'));
});

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  console.log('CORS headers enabled for SharedArrayBuffer support');
});
