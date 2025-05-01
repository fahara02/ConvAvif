const express = require('express');
const app = express();
const path = require('path');

// Required security headers
app.use((req, res, next) => {
  res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
  res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
  next();
});

// Serve static files from root directory
app.use(express.static(__dirname));

// Serve build files from /build
app.use('/build', express.static(path.join(__dirname, 'build')));

// Handle direct requests to index.html
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running:
  - Main page: http://localhost:${PORT}
  - WASM files: http://localhost:${PORT}/build/
  - Press Ctrl+C to stop`);
});