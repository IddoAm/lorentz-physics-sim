cmake_minimum_required(VERSION 3.20)
project(lorentz-sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML — only what we need
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)

add_subdirectory(third_party/sfml)
add_subdirectory(third_party/entt)
add_subdirectory(third_party/glm)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SFML_FIND_SFML OFF)
add_subdirectory(third_party/imgui-sfml)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    EnTT::EnTT
    glm::glm
    ImGui-SFML::ImGui-SFML
)

# Copy SFML DLLs next to the exe on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()