CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
project(Satellites)

add_executable(parallel parallel.c)


# Here is an example syntax how to add compiler options to your build process
# See the project work document on compiler flag syntax on Linux and Windows

# target_compile_options(parallel PRIVATE "add-your-option-here")
# target_compile_options(parallel PRIVATE "add-your-second-option-here")

target_compile_options(parallel PRIVATE "/Qvec-report:2")
target_compile_options(parallel PRIVATE "/fp:fast")
#target_compile_options(parallel PRIVATE "/arch:AVX512")



# UNCOMMENT THESE TO ENABLE OPENMP
#
 find_package(OpenMP REQUIRED)
 target_link_libraries(parallel OpenMP::OpenMP_C)


# UNCOMMENT THESE TO ENABLE OPENCL
# This will also copy the kernel source file parallel.cl (created by you) to the build directory
# The copying command is unfortunately not perfect, as it doesn't redo the copy if you only edit
# the parallel.cl, but leave the parallel.c untouched.
# Because of this, you might need to force 'Rebuild All' to ensure kernel code updates propagate
# to the build directory. If you know enough CMake magic, feel free to fix this and let me know.
#
# find_package(OpenCL REQUIRED)
# target_include_directories(parallel PRIVATE ${OpenCL_INCLUDE_DIRS})
# target_link_libraries(parallel ${OpenCL_LIBRARIES})
# add_custom_command(
#     TARGET parallel POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     "${CMAKE_SOURCE_DIR}/parallel.cl"
#     $<TARGET_FILE_DIR:parallel>
#     VERBATIM)


#set(OpenCL_INCLUDE_DIR "C:/Users/miroa/Downloads/OpenCL-SDK-v2024.10.24-Win-x64/OpenCL-SDK-v2024.10.24-Win-x64/include")
#set(OpenCL_LIBRARY "C:/Users/miroa/Downloads/OpenCL-SDK-v2024.10.24-Win-x64/OpenCL-SDK-v2024.10.24-Win-x64/lib/OpenCL.lib")
#
#find_package(OpenCL REQUIRED)
#include_directories(parallel ${OpenCL_INCLUDE_DIR})
#target_link_libraries(parallel ${OpenCL_LIBRARY})
#add_custom_command(
#    TARGET parallel
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    "${CMAKE_SOURCE_DIR}/parallel.cl"
#    $<TARGET_FILE_DIR:parallel>
#    VERBATIM
#)

set(OPENCL_SDK_DIR "${CMAKE_SOURCE_DIR}/OpenCL-SDK-v2024.10.24-Win-x64")
set(OpenCL_INCLUDE_DIR "${OPENCL_SDK_DIR}/include")
set(OpenCL_LIBRARY "${OPENCL_SDK_DIR}/lib/OpenCL.lib")

find_package(OpenCL REQUIRED)
include_directories(parallel ${OpenCL_INCLUDE_DIR})
target_link_libraries(parallel ${OpenCL_LIBRARY})
add_custom_command(
    TARGET parallel
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/parallel.cl"
    $<TARGET_FILE_DIR:parallel>
    VERBATIM
)


# Find and link SDL2
if (WIN32)
    set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2-2.30.7/cmake")
endif()
find_package(SDL2 REQUIRED)
target_include_directories(parallel PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(parallel ${SDL2_LIBRARIES})

if (WIN32)
# copy the sdl2 .dll file to the same folder as the executable
add_custom_command(
    TARGET parallel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:parallel>
    VERBATIM)
else()
    # Math library shouldn't be linked on Windows, but must be linked on Linux
    target_link_libraries(parallel m)
endif()
