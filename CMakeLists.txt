cmake_minimum_required(VERSION 3.12)

project(el-garden)

include_directories(.)
set(SOURCES
  src/main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_library (LIBJSONC_LIBRARIES
  NAMES libjson-c.a
)

find_library (LIBBSD_LIBRARIES
  NAMES libbsd.a
)

find_path (JSON_H_INCLUDE_DIRS
  NAMES json-c/json.h
)

message(STATUS "LIBJSONC_LIBRARIES: ${LIBJSONC_LIBRARIES}")
message(STATUS "LIBBSD_LIBRARIES: ${LIBBSD_LIBRARIES}")

if(LIBJSONC_LIBRARIES)
  message(STATUS "${LIBJSONC_LIBRARIES} was found")
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBJSONC_LIBRARIES})
else()
  message(FATAL_ERROR "libjson-c.a was not found")
endif()

if(LIBBSD_LIBRARIES)
  message(STATUS "${LIBBSD_LIBRARIES} was found")
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBBSD_LIBRARIES})
else()
  message(FATAL_ERROR "libbsd.a was not found")
endif()

if(JSON_H_INCLUDE_DIRS)
  message(STATUS "${JSON_H_INCLUDE_DIRS} was found")
  include_directories(${JSON_H_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "json.h was not found")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pthread -lrt)
include_directories(include/)
target_link_libraries(${PROJECT_NAME} PRIVATE rt pthread)
