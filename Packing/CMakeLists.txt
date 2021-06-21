# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  Packing
  VERSION 1.0.0
  DESCRIPTION "Classes needed to interface with raw binary data."
  LANGUAGES CXX)

# Setup the library
setup_library(module Packing
              dependencies Framework::Configure
                           Framework::Framework
                           DetDescr::DetDescr)

# Setup the translators library
setup_library(module Packing
              name Translators
              dependencies Packing::Packing)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Packing)
