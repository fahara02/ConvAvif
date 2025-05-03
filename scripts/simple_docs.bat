@echo off
echo Generating simplified ConvAvif API documentation...

REM Create docs/api directory if it doesn't exist
if not exist docs mkdir docs
if not exist docs\api mkdir docs\api

REM Run doxygen with our simplified configuration file
echo Running Doxygen with simplified config...
doxygen Doxyfile.simple

REM Check if index.html exists in GitHub Pages directory
if not exist docs\index.html (
    echo Creating GitHub Pages index.html with link to API docs...
    echo ^<!DOCTYPE html^> > docs\index.html
    echo ^<html lang="en"^> >> docs\index.html
    echo ^<head^> >> docs\index.html
    echo     ^<meta charset="UTF-8"^> >> docs\index.html
    echo     ^<meta name="viewport" content="width=device-width, initial-scale=1.0"^> >> docs\index.html
    echo     ^<title^>ConvAvif Documentation^</title^> >> docs\index.html
    echo     ^<style^> >> docs\index.html
    echo         body { font-family: Arial, sans-serif; line-height: 1.6; margin: 0; padding: 20px; color: #333; } >> docs\index.html
    echo         header { margin-bottom: 20px; border-bottom: 1px solid #eee; padding-bottom: 10px; } >> docs\index.html
    echo         nav { margin-bottom: 20px; } >> docs\index.html
    echo         nav ul { list-style: none; padding: 0; } >> docs\index.html
    echo         nav ul li { display: inline; margin-right: 20px; } >> docs\index.html
    echo         a { color: #0366d6; text-decoration: none; } >> docs\index.html
    echo         a:hover { text-decoration: underline; } >> docs\index.html
    echo         .container { max-width: 1200px; margin: 0 auto; } >> docs\index.html
    echo         footer { margin-top: 40px; border-top: 1px solid #eee; padding-top: 10px; font-size: 0.9em; } >> docs\index.html
    echo     ^</style^> >> docs\index.html
    echo ^</head^> >> docs\index.html
    echo ^<body^> >> docs\index.html
    echo     ^<div class="container"^> >> docs\index.html
    echo         ^<header^> >> docs\index.html
    echo             ^<h1^>ConvAvif Documentation^</h1^> >> docs\index.html
    echo         ^</header^> >> docs\index.html
    echo         ^<nav^> >> docs\index.html
    echo             ^<ul^> >> docs\index.html
    echo                 ^<li^>^<a href="#"^>Home^</a^>^</li^> >> docs\index.html
    echo                 ^<li^>^<a href="api/index.html"^>API Documentation^</a^>^</li^> >> docs\index.html
    echo                 ^<li^>^<a href="https://github.com/fahara02/ConvAvif"^>GitHub Repository^</a^>^</li^> >> docs\index.html
    echo             ^</ul^> >> docs\index.html
    echo         ^</nav^> >> docs\index.html
    echo         ^<main^> >> docs\index.html
    echo             ^<section^> >> docs\index.html
    echo                 ^<h2^>Welcome to ConvAvif^</h2^> >> docs\index.html
    echo                 ^<p^>ConvAvif is a WebAssembly-based library for converting images to AVIF format.^</p^> >> docs\index.html
    echo                 ^<h3^>Key Features:^</h3^> >> docs\index.html
    echo                 ^<ul^> >> docs\index.html
    echo                     ^<li^>Fast conversion of images to AVIF format^</li^> >> docs\index.html
    echo                     ^<li^>WebAssembly implementation for cross-platform compatibility^</li^> >> docs\index.html
    echo                     ^<li^>Support for various quality levels and encoding parameters^</li^> >> docs\index.html
    echo                     ^<li^>Integration with JavaScript applications^</li^> >> docs\index.html
    echo                 ^</ul^> >> docs\index.html
    echo                 ^<p^>^<a href="api/index.html" class="btn"^>View API Documentation^</a^>^</p^> >> docs\index.html
    echo             ^</section^> >> docs\index.html
    echo         ^</main^> >> docs\index.html
    echo         ^<footer^> >> docs\index.html
    echo             ^<p^>&copy; 2025 ConvAvif Developers^</p^> >> docs\index.html
    echo         ^</footer^> >> docs\index.html
    echo     ^</div^> >> docs\index.html
    echo ^</body^> >> docs\index.html
    echo ^</html^> >> docs\index.html
) else (
    REM Add API Documentation link to existing index.html if not already present
    findstr /C:"API Documentation" docs\index.html >nul
    if %ERRORLEVEL% NEQ 0 (
        echo Adding API Documentation link to existing index.html...
        powershell -Command "(Get-Content docs\index.html) -replace '(</nav>|</ul>)', '                <li><a href=\"api/index.html\">API Documentation</a></li>`n$1' | Set-Content docs\index.html"
    )
)

echo Documentation generated successfully!
echo API documentation is available at docs/api/index.html
echo You can now commit and push these changes to update GitHub Pages.
