@echo off
echo Generating ConvAvif API documentation...



REM Create API directory if it doesn't exist
if not exist docs\api mkdir docs\api

REM Check if doxygen is installed and in PATH
where doxygen >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Doxygen is not installed or not in your PATH.
    echo Please install Doxygen from http://www.doxygen.nl/download.html
    echo or use: choco install doxygen.install
    exit /b 1
)

REM Run doxygen with our configuration file
echo Running Doxygen...
doxygen Doxyfile

REM Check if index.html exists, if not, create a basic one
if not exist docs\index.html (
    echo Creating basic index.html...
    echo ^<!DOCTYPE html^> > docs\index.html
    echo ^<html lang="en"^> >> docs\index.html
    echo ^<head^> >> docs\index.html
    echo     ^<meta charset="UTF-8"^> >> docs\index.html
    echo     ^<meta name="viewport" content="width=device-width, initial-scale=1.0"^> >> docs\index.html
    echo     ^<title^>ConvAvif Documentation^</title^> >> docs\index.html
    echo     ^<link rel="stylesheet" href="styles.css"^> >> docs\index.html
    echo ^</head^> >> docs\index.html
    echo ^<body^> >> docs\index.html
    echo     ^<header^> >> docs\index.html
    echo         ^<h1^>ConvAvif Documentation^</h1^> >> docs\index.html
    echo     ^</header^> >> docs\index.html
    echo     ^<nav^> >> docs\index.html
    echo         ^<ul^> >> docs\index.html
    echo             ^<li^>^<a href="#"^>Home^</a^>^</li^> >> docs\index.html
    echo             ^<li^>^<a href="api/index.html"^>API Documentation^</a^>^</li^> >> docs\index.html
    echo         ^</ul^> >> docs\index.html
    echo     ^</nav^> >> docs\index.html
    echo     ^<main^> >> docs\index.html
    echo         ^<section^> >> docs\index.html
    echo             ^<h2^>Welcome to ConvAvif^</h2^> >> docs\index.html
    echo             ^<p^>ConvAvif is a WebAssembly-based library for converting images to AVIF format.^</p^> >> docs\index.html
    echo             ^<p^>^<a href="api/index.html"^>View API Documentation^</a^>^</p^> >> docs\index.html
    echo         ^</section^> >> docs\index.html
    echo     ^</main^> >> docs\index.html
    echo     ^<footer^> >> docs\index.html
    echo         ^<p^>&copy; 2025 ConvAvif Developers^</p^> >> docs\index.html
    echo     ^</footer^> >> docs\index.html
    echo ^</body^> >> docs\index.html
    echo ^</html^> >> docs\index.html
) else (
    REM Add API Documentation link to existing index.html if not already present
    findstr /C:"API Documentation" docs\index.html >nul
    if %ERRORLEVEL% NEQ 0 (
        echo Adding API Documentation link to existing index.html...
        powershell -Command "(Get-Content docs\index.html) -replace '(</nav>)', '        <li><a href=\"api/index.html\">API Documentation</a></li>`n$1' | Set-Content docs\index.html"
    )
)

echo Documentation generated successfully!
echo API documentation is available at docs/api/index.html
echo You can now commit and push these changes to update GitHub Pages.
