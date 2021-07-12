# install mojo to make sure GDML module names are absolute paths pointing to installed files
set(detector_data ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(GLOB detector_dirs RELATIVE ${detector_data} ${detector_data}/*) 
#loop through all the detector versions
foreach(detector_dir ${detector_dirs})
  set(detector_path ${detector_data}/${detector_dir})
  if(IS_DIRECTORY ${detector_path})
    #this is a directory
    #   go through detector.gdml and replace the relative paths with aboslute paths to the installation
    set(detector_file ${detector_path}/detector.gdml)
    # read in detector.gdml contents
    file(READ ${detector_file} detector_contents)
    # get matches for other gdml files
    string(REGEX MATCHALL \"[A-Za-z_]*.gdml\" gdml_module_matches "${detector_contents}")
    foreach (gdml_module_match ${gdml_module_matches}) 
      #replace relative paths to absolute install paths for these gdml
      string(REPLACE "\"" "" gdml_module ${gdml_module_match})
      set(gdml_module_path ${CMAKE_INSTALL_PREFIX}/data/detectors/${detector_dir}/${gdml_module})
      string(REPLACE ${gdml_module} ${gdml_module_path} detector_contents "${detector_contents}")
    endforeach()
    # get matche(s) for the magnetic field map
    string(REGEX MATCHALL \"Bmap.*dat\" mag_map_matches "${detector_contents}")
    foreach (mag_map_match ${mag_map_matches})
      #replace relative paths to absolute install paths for these mag field maps
      string(REPLACE "\"" "" mag_map_name ${mag_map_match})
      set(mag_map_path ${CMAKE_INSTALL_PREFIX}/data/fieldmap/${mag_map_name})
      string(REPLACE ${mag_map_name} ${mag_map_path} detector_contents "${detector_contents}")
    endforeach()
    # done searching and replacing --> write out file and install the new copy
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/data/${detector_dir}/detector.gdml "${detector_contents}")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/data/${detector_dir}/detector.gdml DESTINATION data/detectors/${detector_dir} 
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
    # install other gdml files without any modifications
    file(GLOB detector_files ${detector_path}/*.gdml)
    foreach(detector_file ${detector_files})
      get_filename_component(detector_file_name ${detector_file} NAME)  
      if(NOT ${detector_file_name} MATCHES "detector.gdml") 
        install(FILES ${detector_file} DESTINATION data/detectors/${detector_dir} 
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
      endif()
    endforeach()
  endif()
endforeach()

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Detectors)
