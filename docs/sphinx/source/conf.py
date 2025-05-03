# Configuration file for the Sphinx documentation builder.

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
breathe_projects = {
    "ConvAvif": "D:\_0_DEV_SOFTWARE\WASM\ConvAvif\docs\doxygen_xml",
}
breathe_default_project = "ConvAvif"
breathe_domain_by_extension = {
    "h": "cpp",
    "hpp": "cpp",
    "c": "c",
    "cpp": "cpp",
    "ts": "js",
}

# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
