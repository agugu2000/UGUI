cmake_minimum_required(VERSION 3.16)
project(ugui_sim C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---- Put all binaries in build/ root ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
foreach(cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR})
endforeach()

# ================================================================================
# SDL2: extract deps/*.zip to build/_deps/ then add_subdirectory
# ================================================================================
set(SDL_ZIP      "${CMAKE_SOURCE_DIR}/deps/SDL-release-2.32.10.zip")
set(SDL_DEPS_DIR "${CMAKE_BINARY_DIR}/_deps")
set(SDL_SRC_DIR  "${SDL_DEPS_DIR}/SDL")

if(NOT EXISTS "${SDL_SRC_DIR}/CMakeLists.txt")
    if(NOT EXISTS "${SDL_ZIP}")
        message(FATAL_ERROR "SDL zip not found: ${SDL_ZIP}")
    endif()

    message(STATUS "Extracting SDL from ${SDL_ZIP} ...")
    file(MAKE_DIRECTORY "${SDL_DEPS_DIR}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf "${SDL_ZIP}"
        WORKING_DIRECTORY "${SDL_DEPS_DIR}"
        RESULT_VARIABLE SDL_UNZIP_RESULT
    )
    if(NOT SDL_UNZIP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract SDL zip (code ${SDL_UNZIP_RESULT})")
    endif()

    # zip usually has a top-level dir like SDL-*/ or SDL2-*/, rename it to SDL/
    if(NOT EXISTS "${SDL_SRC_DIR}/CMakeLists.txt")
        file(GLOB SDL_INNER "${SDL_DEPS_DIR}/SDL*")
        foreach(candidate IN LISTS SDL_INNER)
            if(EXISTS "${candidate}/CMakeLists.txt")
                file(RENAME "${candidate}" "${SDL_SRC_DIR}")
                break()
            endif()
        endforeach()
    endif()

    # Fallback: if no inner directory, use _deps root directly
    if(NOT EXISTS "${SDL_SRC_DIR}/CMakeLists.txt")
        if(EXISTS "${SDL_DEPS_DIR}/CMakeLists.txt")
            set(SDL_SRC_DIR "${SDL_DEPS_DIR}")
        else()
            message(FATAL_ERROR "Cannot locate SDL source after extraction in ${SDL_DEPS_DIR}")
        endif()
    endif()
endif()

# ---- Embedded SDL2 (static lib only, no tests) ----
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
add_subdirectory("${SDL_SRC_DIR}" "${SDL_DEPS_DIR}/SDL-build" EXCLUDE_FROM_ALL)

# ================================================================================
# uGUI: core sources + simulator
# ================================================================================
set(UGUI_CORE_SRC
    ugui.c
    ugui_button.c
    ugui_checkbox.c
    ugui_textbox.c
    ugui_progress.c
    ugui_image.c
    font_8x8.c
    SIMSUN2_13X13.c
)

set(UGUI_SIM_SRC
    ugui_sim.c
    ugui_sim_sdl.c
)

add_executable(ugui_sim ${UGUI_CORE_SRC} ${UGUI_SIM_SRC})

target_include_directories(ugui_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ---- Link SDL2 static library ----
target_link_libraries(ugui_sim PRIVATE SDL2::SDL2-static)

# ---- Windows (MinGW) ----
if(WIN32)
    target_link_libraries(ugui_sim PRIVATE
        mingw32
        SDL2::SDL2main
        winmm
        gdi32
        ole32
        oleaut32
        imm32
        version
        uuid
        setupapi
    )
endif()

# ---- Linux ----
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(ugui_sim PRIVATE
        dl
        m
        Threads::Threads
    )
endif()

# ================================================================================
# Strip symbols to reduce executable size
# ================================================================================
if(MINGW)
    add_custom_command(TARGET ugui_sim POST_BUILD
        COMMAND strip $<TARGET_FILE:ugui_sim>
        COMMENT "Stripping symbols from executable")
endif()

if(UNIX)
    add_custom_command(TARGET ugui_sim POST_BUILD
        COMMAND strip $<TARGET_FILE:ugui_sim>
        COMMENT "Stripping symbols from executable")
endif()