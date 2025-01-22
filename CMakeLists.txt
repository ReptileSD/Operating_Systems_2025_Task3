cmake_minimum_required(VERSION 3.10)

project(CrossPlatformSync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(main 
    src/main.cpp 
    src/logger.cpp 
    src/counter.cpp
)

add_executable(child_program_1 
    src/child_program_1.cpp 
    src/logger.cpp 
    src/counter.cpp 
)

add_executable(child_program_2 
    src/child_program_2.cpp 
    src/logger.cpp 
    src/counter.cpp 
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

if(MSVC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()
