cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)
include(CMakePrintHelpers)
project(OpenDynaFlash VERSION 0.1 LANGUAGES C CXX)
find_package(Python 3.9 COMPONENTS Interpreter Development NumPy REQUIRED)
################ nanobind boilerplate
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
find_package(nanobind CONFIG REQUIRED)
################ dyna flash python bindings
nanobind_add_module(OpenDynaFlash 
    src/OpenDynaFlash.cpp
)
target_compile_definitions(OpenDynaFlash PRIVATE PYTHON_BINDINGS_BUILD)
target_include_directories(OpenDynaFlash PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(OpenDynaFlash PRIVATE "${PROJECT_SOURCE_DIR}/official_API/include")
target_link_libraries(OpenDynaFlash PRIVATE "${PROJECT_SOURCE_DIR}/official_API/lib/DynaFlash200.lib")

################ examples
add_executable(example 
    src/OpenDynaFlash.cpp
    examples/example.cpp
)
# target_link_libraries(example PRIVATE ${OpenCV_LIBS})
target_include_directories(example PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(example PRIVATE "${PROJECT_SOURCE_DIR}/official_API/include")
target_link_libraries(example PRIVATE "${PROJECT_SOURCE_DIR}/official_API/lib/DynaFlash200.lib")