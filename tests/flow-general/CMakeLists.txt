# Test executables
	
add_executable(e_testflow_wallbcs testd_wallbcs.cpp testwallbcs.cpp)
target_link_libraries(e_testflow_wallbcs fvens_base)

add_executable(runtest_res_hist test_res_hist.cpp)

# List of control files
set(CONTROL_FILES test_res_hist.sh)

# Process them to include CMake variables
foreach(file ${CONTROL_FILES})
  message(STATUS "Configuring control file ${file}")
  configure_file(${file} ${file})
endforeach(file)

# Tests

add_test(NAME SpatialFlow_BC_Walls WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl wall_boundaries
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)

add_test(NAME SpatialFlow_Walltest_HLLC WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux HLLC
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)
add_test(NAME SpatialFlow_Walltest_Roe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux ROE
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)
add_test(NAME SpatialFlow_Walltest_AUSM WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND  ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux AUSM
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)
add_test(NAME SpatialFlow_Walltest_AUSMPlus WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND  ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux AUSMPLUS
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)
add_test(NAME SpatialFlow_Walltest_HLL WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux HLL
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)
add_test(NAME SpatialFlow_Walltest_LLF WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} e_testflow_wallbcs
  ${CMAKE_CURRENT_SOURCE_DIR}/test.ctrl
  numerical_flux LLF
  --mesh_file ${CMAKE_CURRENT_SOURCE_DIR}/../common-input/testperiodic.msh)

add_test(NAME PseudotimeFlow_exception_nanorinf WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${SEQEXEC} ${SEQTASKS} ../e_testflow_pseudotime
  ${CMAKE_CURRENT_SOURCE_DIR}/testexception.ctrl
  --test_type exception_nanorinf
  -options_file ${CMAKE_CURRENT_SOURCE_DIR}/testexception.solverc
  --mesh_file ${CMAKE_SOURCE_DIR}/testcases/visc-naca0012/grids/NACA0012_lam_hybrid_1.msh)

