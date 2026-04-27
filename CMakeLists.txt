cmake_minimum_required(VERSION 3.16)
project(InfiniteVoxelCaveWorld LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Sources ---
set(SOURCES
    main.cpp
    Chunk.cpp
    ChunkManager.cpp
    stb_image.cpp
    glad.c
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include paths ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
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
        ${CMAKE_SOURCE_DIR}/stone_16.png $<TARGET_FILE_DIR:${PROJECT_NAME}>/stone_16.png
)
