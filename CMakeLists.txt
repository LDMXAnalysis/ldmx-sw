
# Set the minimum version of CMake that's required 
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(LDMX_SW VERSION 2.1.0
                DESCRIPTION "The Light Dark Matter eXperiment simulation and reconstruction framework." 
                LANGUAGES CXX    
)

# Load additional macros used by this project. 
list(APPEND CMAKE_MODULE_PATH ${LDMX_SW_SOURCE_DIR}/cmake/)

# Load the BuildMacros module. If loaded correctly, the variable 
# 'build_macros_found' will be set to the path of the module.  Otherwise, it
# is set to NOTFOUND.
include(BuildMacros RESULT_VARIABLE build_macros_found)

# If an install location hasn't been set via CMAKE_INSTALL_PREFIX, set it to 
# a reasonable default ($PWD/install).  
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install CACHE PATH "" FORCE)
    message(STATUS "Install directory set to ${CMAKE_INSTALL_PREFIX}")
endif()

# If a user is building outside of a Docker or Singularity environment, 
# warn them. 
if (NOT EXISTS /.dockerenv AND NOT EXISTS /singularity)
    message(WARNING "You are not inside a container; you may be working in an untested environment.")
endif()

# Set the python installation path
set(PYTHON_PACKAGE_NAME LDMX)

# Set the default release type to "Release". If a release type is specified 
# at the command line, it's respected.
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

# Clear any variables cached during previous configuration cycles. 
clear_cache_variables()

# The following types of builds are allowed
#
# * Recon - In addition to building the framework, build the modules needed 
# by the reconstruction.  
# * Sim - Only build the modules necessary to run the simulation.
# * Eve - Build the event display only.
#
# By default, all modules are built.
option(BUILD_RECON_ONLY     "Build the modules necessary to run the reconstruction." OFF)
option(BUILD_SIM_ONLY       "Build the modules necessary to run the simulation."     OFF)

if(NOT BUILD_RECON_ONLY AND NOT BUILD_SIM_ONLY) 
    set(BUILD_ALL ON)
    message(STATUS "Building all modules.")
endif()

# Start by building all of the classes needed for building the event bus and 
# ROOT dictionary. This is on by forcing the global "BUILD_EVENT_ONLY" option
# here.
set(BUILD_EVENT_ONLY ON CACHE BOOL "Build the SimCore library." FORCE)

add_subdirectory(Recon ${CMAKE_BINARY_DIR}/ReconEvent)
add_subdirectory(SimCore ${CMAKE_BINARY_DIR}/SimCoreEvent)
add_subdirectory(Ecal ${CMAKE_BINARY_DIR}/EcalEvent)
add_subdirectory(Hcal ${CMAKE_BINARY_DIR}/HcalEvent)
add_subdirectory(TrigScint ${CMAKE_BINARY_DIR}/TrigScintEvent)

# Once the event libraries have been built, turn off the global option. 
set(BUILD_EVENT_ONLY OFF CACHE BOOL "Build the SimCore library." FORCE)

# The framework has all the backend code needed to run all processors.
add_subdirectory(Framework)

# This module contains all detector service related code.  Needed by recon. 
add_subdirectory(DetDescr)

if(BUILD_ALL OR BUILD_RECON_ONLY)

    message(STATUS "Building the modules necessary for the reconstruction.")

    # Conditions services
    add_subdirectory(Conditions)

    # The tools module contains tools generic enough to be used by other modules.
    add_subdirectory(Tools)

    # This module contains ECal reconstruction code.
    add_subdirectory(Ecal)

    # Currently, this module contains processors from several subsystems.
    add_subdirectory(Recon)

    # Data quality management plots.
    add_subdirectory(DQM)

    # This module contains HCal reconstruction code.
    add_subdirectory(Hcal)

    # This module contains all things trigger scintillator
    add_subdirectory(TrigScint)

    # This module contains trigger algorithms.
    add_subdirectory(Trigger/Algo)

endif()

if(BUILD_ALL OR BUILD_SIM_ONLY)

    message(STATUS "Building the modules necessary for the simulation.")

    # SimCore allows the simulation to be run without biasing
    add_subdirectory(SimCore)

    # Adding the Biasing module, allows the use of biasing in the simulation
    add_subdirectory(Biasing)

    # Add the Detectors submodule.  This has no dependecies.
    add_subdirectory(Detectors)

    # Add the magnetic field map directory.  This simply installs the B-field maps.
    add_subdirectory(MagFieldMap)

endif()

# Build all test.  The test are assumed to reside inside of the directory 
# "test" within each module.  The main
build_test()

# The EventDisplay is stand-a-lone and _needs_ to be after everything else
# so that it can clear the cache variables and construct its own dictionary.
option(BUILD_EVE "Build the event display as well as other parts." OFF)
if(BUILD_EVE)
    add_subdirectory(EventDisplay)
endif()
