cmake_minimum_required(VERSION 3.10)
project(FlagsK4FreePartitions CXX)

option(USE_OPENMP "Enable OpenMP" ON)

if(USE_OPENMP)
    if(APPLE)
        find_package(OpenMP)
        if(NOT OpenMP_FOUND)
            # libomp 15.0+ from brew is keg-only, so have to search in other locations.
            # See https://github.com/Homebrew/homebrew-core/issues/112107#issuecomment-1278042927.
            execute_process(COMMAND brew --prefix libomp
                            OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
                            OUTPUT_STRIP_TRAILING_WHITESPACE)
            set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
            set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
            set(OpenMP_C_LIB_NAMES omp)
            set(OpenMP_CXX_LIB_NAMES omp)
            set(OpenMP_omp_LIBRARY ${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib)
            find_package(OpenMP REQUIRED)
        endif()
    else()
        find_package(OpenMP REQUIRED)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(COMMON_FLAG_SOURCES
  src/option-parser.cpp
  src/solution-processor.cpp
  src/flag-generator.cpp
  src/flag.cpp
  src/utils.cpp
  src/cache.cpp
  src/solver.cpp
)

add_executable(max-2 problems/max-2.cpp ${COMMON_FLAG_SOURCES})
if(USE_OPENMP)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_link_libraries(max-2 PUBLIC OpenMP::OpenMP_CXX)
  endif()
endif()
target_include_directories(max-2 PRIVATE src)
target_compile_definitions(max-2 PRIVATE V=5 G_COLORED_EDGES=3 G_ORIENTED_EDGES=1 G_ORIENTED_EDGES_UNORIENTED_COLORS=2 G_COLORED_VERTICES=3)

add_executable(max-2-less-colors problems/max-2.cpp ${COMMON_FLAG_SOURCES})
if(USE_OPENMP)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_link_libraries(max-2-less-colors PUBLIC OpenMP::OpenMP_CXX)
  endif()
endif()
target_include_directories(max-2-less-colors PRIVATE src)
target_compile_definitions(max-2-less-colors PRIVATE V=6 G_COLORED_EDGES=3 G_ORIENTED_EDGES=1 G_ORIENTED_EDGES_UNORIENTED_COLORS=2 G_COLORED_VERTICES=2)

foreach(N RANGE 1 4)
  add_custom_target(max-2-case-${N}-solve-approximate
    COMMAND ./bin/max-2 -l 3 -c ${N}
    DEPENDS max-2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endforeach()

foreach(N RANGE 5 8)
  add_custom_target(max-2-case-${N}-solve-approximate
    COMMAND ./bin/max-2-less-colors -l 3 -c ${N}
    DEPENDS max-2-less-colors
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endforeach()
