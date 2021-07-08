# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(
  HexReformat
  VERSION 1.0.0
  DESCRIPTION "Translation layer between hexactrl-sw and ldmx-sw."
  LANGUAGES CXX)

# Search for Boost
find_package(
  Boost REQUIRED
  COMPONENTS serialization
             program_options)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

# Make a library
add_library(HexaReformat 
            SHARED ${PROJECT_SOURCE_DIR}/src/HGCROCv2RawData.cxx
                   ${PROJECT_SOURCE_DIR}/src/RawEventFile.cxx)
target_include_directories(HexaReformat 
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(HexaReformat 
                      PUBLIC Boost::serialization 
                             ROOT::Core 
                             ROOT::TreePlayer)
install(TARGETS HexaReformat 
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Add the executable
add_executable(hexa-reformat ${PROJECT_SOURCE_DIR}/src/hexa_reformat.cxx)
target_link_libraries(hexa-reformat 
                      PUBLIC HexaReformat 
                             Boost::program_options)
install(TARGETS hexa-reformat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

