
cmake_minimum_required(VERSION 3.22)

# Feel free to change the standard option (-std=...)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++23 -fdiagnostics-color -Werror -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Winit-self -Wlogical-op -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wundef -Wno-unused -Wunused-variable -Wno-variadic-macros -Wno-parentheses -Wno-unknown-pragmas -Wwrite-strings -Wswitch -Wunused-parameter -Wfloat-equal -Wnon-virtual-dtor -Weffc++ -Wdate-time -Wno-trigraphs -fdiagnostics-show-option")

project(shisen-sho-resolver)

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

include_directories(
    ${ImageMagick_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    shisen-sho-resolver.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${ImageMagick_LIBRARIES}
)

# vim: ts=4 sw=4 et
