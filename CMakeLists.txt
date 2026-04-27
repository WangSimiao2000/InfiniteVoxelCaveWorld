cmake_minimum_required(VERSION 3.16)
project(InfiniteVoxelCaveWorld LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Sources ---
set(SOURCES
    src/main.cpp
    src/Chunk.cpp
    src/ChunkManager.cpp
    third_party/stb/stb_image.cpp
    third_party/glad/glad.c
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_impl_glfw.cpp
    third_party/imgui/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include paths ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/glad
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/OpenGL/include
)

# --- Find system libraries ---
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    pthread
)

# --- Copy runtime assets next to binary ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/textures/stone_16.png $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures/stone_16.png
)
