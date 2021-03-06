macro(generate_message_properties_map)
set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/message_properties_map")
file(MAKE_DIRECTORY "${OUTPUT_DIR}/src")
file(MAKE_DIRECTORY "${OUTPUT_DIR}/include")
set(
  CPP_AND_HPP
  "${OUTPUT_DIR}/src/message_properties_map.cpp"
  "${OUTPUT_DIR}/include/message_properties_map.hpp"
)
set(GENERATOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_message_properties_map.py")
add_custom_command(
  OUTPUT ${CPP_AND_HPP}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENERATOR_SCRIPT}
  COMMENT "Generating C++ code for message_properties_map"
  DEPENDS ${GENERATOR_SCRIPT}
  WORKING_DIRECTORY ${OUTPUT_DIR}
)
endmacro()