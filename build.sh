#!/usr/bin/env bash
# ------------------------------------------------------------------------------
# uGUI Simulator build script
# Supports: Linux GCC, Windows MinGW
# Usage:
#   ./build.sh          # configure + build
#   ./build.sh clean    # remove build directory
#   ./build.sh rebuild  # clean + build
# ------------------------------------------------------------------------------

set -e

# ---- Config ----
BUILD_DIR="build"
BUILD_TYPE="Release"

# ---- Detect platform ----
case "$(uname -s)" in
    Linux*)
        PLATFORM="Linux"
        GENERATOR="Unix Makefiles"
        ;;
    MINGW*)
        PLATFORM="Windows/MinGW"
        GENERATOR="MinGW Makefiles"
        ;;
    *)
        PLATFORM="Unknown"
        GENERATOR="Unix Makefiles"
        ;;
esac

echo "==> Platform  : ${PLATFORM}"
echo "==> Generator : ${GENERATOR}"
echo "==> Build dir : ${BUILD_DIR}"
echo "==> Build type: ${BUILD_TYPE}"
echo

# ---- Helpers ----
clean_build() {
    echo "==> Cleaning ${BUILD_DIR} ..."
    rm -rf "${BUILD_DIR}"
    echo "==> Clean done."
}

configure_build() {
    echo "==> Configuring CMake ..."
    cmake -S . -B "${BUILD_DIR}" \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++
}

compile_build() {
    echo "==> Building ..."
    cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --parallel
}

# ---- Main ----
case "$1" in
    clean)
        clean_build
        ;;
    rebuild)
        clean_build
        configure_build
        compile_build
        ;;
    ""|build)
        if [ ! -d "${BUILD_DIR}" ]; then
            configure_build
        fi
        compile_build
        ;;
    *)
        echo "Usage: $0 [clean|build|rebuild]"
        exit 1
        ;;
esac

# ---- Report executable ----
echo
echo "==> Done."
if [ "${PLATFORM}" = "Windows/MinGW" ]; then
    echo "    Executable: ${BUILD_DIR}/ugui_sim.exe"
else
    echo "    Executable: ${BUILD_DIR}/ugui_sim"
fi