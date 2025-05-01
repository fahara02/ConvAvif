const express = require('express');
const path = require('path');
const app = express();

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

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}
  - Project root: ${PROJECT_ROOT}
  - Serving:
    - /build → ${path.join(PROJECT_ROOT, 'build')}
    - /asset → ${path.join(PROJECT_ROOT, 'asset')}
    - /dist  → ${path.join(PROJECT_ROOT, 'dist')}
  Press Ctrl+C to stop`);
});
