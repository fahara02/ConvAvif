#!/usr/bin/env python3
"""
Generate Breathe documentation for ConvAvif

This script:
1. Updates the Doxyfile to generate XML output
2. Runs Doxygen to generate XML
3. Sets up a Sphinx project with Breathe
4. Builds the Sphinx documentation directly in docs/api
"""

import os
import sys
import shutil
import subprocess
from pathlib import Path

# Project paths
PROJECT_ROOT = Path(__file__).parent.absolute()
DOCS_DIR = PROJECT_ROOT / "docs"
DOCS_API_DIR = DOCS_DIR / "api"
DOCS_BUILD_DIR = DOCS_API_DIR / "_build"
DOXYGEN_XML_DIR = DOCS_DIR / "doxygen_xml"

def ensure_dir(directory):
    """Ensure a directory exists."""
    directory.mkdir(parents=True, exist_ok=True)

def backup_landing_page():
    """Backup the main landing page before running Doxygen."""
    landing_page = DOCS_DIR / "index.html"
    backup_file = DOCS_DIR / "index.html.backup"
    
    if landing_page.exists():
        print("Backing up main landing page...")
        shutil.copy(landing_page, backup_file)
        return True
    return False

def restore_landing_page():
    """Restore the main landing page after Doxygen runs."""
    landing_page = DOCS_DIR / "index.html"
    backup_file = DOCS_DIR / "index.html.backup"
    
    if backup_file.exists():
        print("Restoring main landing page...")
        shutil.copy(backup_file, landing_page)
        backup_file.unlink()  # Remove the backup after restoring

def update_doxyfile():
    """Update the Doxyfile to enable XML output."""
    doxyfile_path = PROJECT_ROOT / "Doxyfile"
    temp_path = PROJECT_ROOT / "Doxyfile.temp"
    
    # Ensure XML directory exists
    ensure_dir(DOXYGEN_XML_DIR)
    
    # Settings to update or add
    xml_settings = {
        "GENERATE_XML": "YES",
        "XML_OUTPUT": str(DOXYGEN_XML_DIR),
        "OUTPUT_DIRECTORY": str(DOCS_DIR),
        "HTML_OUTPUT": "api_temp",  # Change HTML output to a temp directory to avoid overwriting main landing page
    }
    
    # Settings that might cause problems
    problematic_settings = [
        "CLANG_ASSISTED_PARSING",
        "CLANG_OPTIONS"
    ]
    
    with open(doxyfile_path, 'r') as infile, open(temp_path, 'w') as outfile:
        for line in infile:
            # Skip problematic settings
            if any(line.strip().startswith(setting) for setting in problematic_settings):
                continue
                
            # Check if the current line starts with any of our settings
            found = False
            for setting, value in xml_settings.items():
                if line.strip().startswith(setting):
                    outfile.write(f"{setting.ljust(25)} = {value}\n")
                    found = True
                    break
            
            # If not one of our settings, write the original line
            if not found:
                outfile.write(line)
        
        # If any settings weren't found, add them at the end
        for setting, value in xml_settings.items():
            if f"{setting} =" not in open(doxyfile_path, 'r').read():
                outfile.write(f"\n{setting.ljust(25)} = {value}")
    
    # Replace the original Doxyfile with our updated one
    shutil.move(str(temp_path), str(doxyfile_path))
    
    print("Updated Doxyfile with XML output settings")

def run_doxygen():
    """Run Doxygen to generate XML output."""
    # Ensure all output directories exist before running Doxygen
    ensure_dir(DOCS_DIR)
    ensure_dir(DOXYGEN_XML_DIR)
    
    # Backup landing page
    had_landing_page = backup_landing_page()
    
    result = subprocess.run(["doxygen"], cwd=str(PROJECT_ROOT), 
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
                             text=True, check=False)
    
    # Restore landing page
    if had_landing_page:
        restore_landing_page()
    
    if result.returncode != 0:
        print("Doxygen Warning/Error (continuing anyway):", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        
        # Check if the XML output was actually generated despite errors
        if not os.path.exists(DOXYGEN_XML_DIR / "index.xml"):
            print("Fatal error: XML output was not generated. Exiting.", file=sys.stderr)
            sys.exit(1)
    
    # Clean up temporary HTML output 
    api_temp_dir = DOCS_DIR / "api_temp"
    if api_temp_dir.exists():
        shutil.rmtree(api_temp_dir)
    
    print("Doxygen XML generation completed")

def clean_api_dir():
    """Clean the existing docs/api directory to prepare for new Sphinx output."""
    if DOCS_API_DIR.exists():
        print(f"Cleaning docs/api directory...")
        
        # Save placeholder.svg if it exists in asset subdirectory
        asset_dir = DOCS_API_DIR / "asset"
        placeholder_svg = asset_dir / "placeholder.svg" if asset_dir.exists() else None
        if placeholder_svg and placeholder_svg.exists():
            temp_placeholder = DOCS_DIR / "placeholder.svg.backup"
            shutil.copy(placeholder_svg, temp_placeholder)
        
        # Empty the directory
        for item in DOCS_API_DIR.iterdir():
            if item.is_file():
                item.unlink()
            elif item.is_dir() and item.name != "asset":
                shutil.rmtree(item)
        
        # Restore placeholder.svg if it was backed up
        if 'temp_placeholder' in locals() and temp_placeholder.exists():
            ensure_dir(asset_dir)
            shutil.copy(temp_placeholder, placeholder_svg)
            temp_placeholder.unlink()
    else:
        ensure_dir(DOCS_API_DIR)

def setup_sphinx():
    """Set up a Sphinx project with Breathe configured."""
    ensure_dir(DOCS_API_DIR)
    sphinx_source_dir = DOCS_API_DIR / "source"
    ensure_dir(sphinx_source_dir)
    
    # Create a basic conf.py
    conf_path = sphinx_source_dir / "conf.py"
    with open(conf_path, 'w') as f:
        f.write(f"""# Configuration file for the Sphinx documentation builder.

import os
import sys

# -- Project information -----------------------------------------------------
project = 'ConvAvif'
copyright = '2025, ConvAvif Team'
author = 'ConvAvif Team'

# -- General configuration ---------------------------------------------------
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'breathe',
    'sphinx_rtd_theme',
]

templates_path = ['_templates']
exclude_patterns = []

# -- Breathe configuration -------------------------------------------------
breathe_projects = {{
    "ConvAvif": "{str(DOXYGEN_XML_DIR)}",
}}
breathe_default_project = "ConvAvif"
breathe_domain_by_extension = {{
    "h": "cpp",
    "hpp": "cpp",
    "c": "c",
    "cpp": "cpp",
    "ts": "js",
}}

# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

# -- Other Sphinx settings --------------------------------------------------
add_module_names = False
nitpicky = True
nitpick_ignore = [('cpp:identifier', 'std::string')]
cpp_index_common_prefix = ['avif::']
""")
    
    # Create index.rst
    index_path = sphinx_source_dir / "index.rst"
    with open(index_path, 'w') as f:
        f.write("""
Welcome to ConvAvif Documentation
======================================

AVIF Image Conversion Library with WebAssembly Support

ConvAvif is a C++/WebAssembly library for converting images to the AVIF format. 
It provides both a C++ API and JavaScript/TypeScript bindings for use in web applications.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   library_root
   classes
   files
   namespaces

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
""")
    
    # Create API documents directly in the source directory
    # Create library_root.rst
    with open(sphinx_source_dir / "library_root.rst", 'w') as f:
        f.write("""
Library Overview
===============

ConvAvif provides a comprehensive API for converting images to the AVIF format.
The library is designed to be used directly from C++ or through WebAssembly bindings in JavaScript/TypeScript.

Key Features
-----------

* Support for JPEG and PNG input formats
* Configurable encoding parameters
* Multiple codec options (AOM, SVT)
* Quality and performance tuning options

Key Components
-------------

* **EncodeConfig**: Configuration for the AVIF encoder
* **ImageGuru**: Image format detection and conversion
* **Error**: Error handling and reporting
* **ImageBuffer**: Image data buffer management

.. doxygenindex::
   :project: ConvAvif
""")
    
    # Create classes.rst with specific references to the project's classes
    with open(sphinx_source_dir / "classes.rst", 'w') as f:
        f.write("""
Classes
=======

This section describes the classes and structures provided by the ConvAvif library.

Core Classes
-----------

EncodeConfig
~~~~~~~~~~~

.. doxygenstruct:: EncodeConfig
   :members:
   :undoc-members:

ImageGuru
~~~~~~~~~

.. doxygenclass:: ImageGuru
   :members:
   :undoc-members:

Error
~~~~~

.. doxygenclass:: Error
   :members:
   :undoc-members:

Speed
~~~~~

.. doxygenstruct:: Speed
   :members:
   :undoc-members:

SpeedRange
~~~~~~~~~~

.. doxygenstruct:: SpeedRange
   :members:
   :undoc-members:

ImageBuffer
~~~~~~~~~~

.. doxygenclass:: ImageBuffer
   :members:
   :undoc-members:

Enumerations
-----------

CodecChoice
~~~~~~~~~~

.. doxygenenum:: CodecChoice

Tune
~~~~

.. doxygenenum:: Tune

SpeedPreset
~~~~~~~~~~

.. doxygenenum:: SpeedPreset

ConverterError
~~~~~~~~~~~~~

.. doxygenenum:: ConverterError
""")
    
    # Create files.rst with specific references to the project's files
    with open(sphinx_source_dir / "files.rst", 'w') as f:
        f.write("""
Files
=====

This section describes the source files in the ConvAvif library.

Header Files
-----------

encoder_config.h
~~~~~~~~~~~~~~~

.. doxygenfile:: encoder_config.h
   :project: ConvAvif

error.h
~~~~~~

.. doxygenfile:: error.h
   :project: ConvAvif

imageGuru.h
~~~~~~~~~~

.. doxygenfile:: imageGuru.h
   :project: ConvAvif

imagebuffer.h
~~~~~~~~~~~~

.. doxygenfile:: imagebuffer.h
   :project: ConvAvif

imageconverter.h
~~~~~~~~~~~~~~~

.. doxygenfile:: imageconverter.h
   :project: ConvAvif

result.h
~~~~~~~

.. doxygenfile:: result.h
   :project: ConvAvif

avif_helper.h
~~~~~~~~~~~~

.. doxygenfile:: avif_helper.h
   :project: ConvAvif

constants.h
~~~~~~~~~~

.. doxygenfile:: constants.h
   :project: ConvAvif

Implementation Files
------------------

error.cpp
~~~~~~~~

.. doxygenfile:: error.cpp
   :project: ConvAvif

bind.cpp
~~~~~~~

.. doxygenfile:: bind.cpp
   :project: ConvAvif

imageconverter.cpp
~~~~~~~~~~~~~~~~~

.. doxygenfile:: imageconverter.cpp
   :project: ConvAvif
""")
    
    # Create namespaces.rst
    with open(sphinx_source_dir / "namespaces.rst", 'w') as f:
        f.write("""
Namespaces
==========

This section describes the namespaces in the ConvAvif library.

.. doxygennamespace:: std
   :project: ConvAvif
""")
    
    print("Sphinx project set up with Breathe configuration")

def build_sphinx():
    """Build the Sphinx documentation with Breathe directly into docs/api."""
    # Build directory is the api directory itself, not a subdirectory
    build_dir = DOCS_API_DIR
    source_dir = DOCS_API_DIR / "source"
    ensure_dir(build_dir)
    
    result = subprocess.run(
        ["sphinx-build", "-b", "html", str(source_dir), str(build_dir)],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True, check=False
    )
    
    if result.returncode != 0:
        print("Sphinx build warning/error (continuing anyway):", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
    
    print(f"Sphinx documentation built successfully at: {build_dir}")
    print(f"Open {build_dir / 'index.html'} in your browser to view the documentation")
    
    # Cleanup: Remove source directory after successful build
    if (DOCS_API_DIR / "source").exists() and (DOCS_API_DIR / "index.html").exists():
        shutil.rmtree(DOCS_API_DIR / "source")
        print("Cleaned up source directory after successful build")

def main():
    """Main function to coordinate the documentation generation process."""
    print("Generating Breathe documentation for ConvAvif...")
    
    # Ensure docs directories exist
    ensure_dir(DOCS_DIR)
    ensure_dir(DOXYGEN_XML_DIR)
    
    # Update Doxyfile for XML output
    update_doxyfile()
    
    # Run Doxygen to generate XML
    run_doxygen()
    
    # Clean up existing API directory
    clean_api_dir()
    
    # Set up Sphinx with Breathe
    setup_sphinx()
    
    # Build the Sphinx documentation
    build_sphinx()
    
    print("Documentation generation complete!")

if __name__ == "__main__":
    main()
