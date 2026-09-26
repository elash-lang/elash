#!/usr/bin/env bash
cd "$("$(dirname "$0")"/directory.sh "aarch64-linux")" || exit

export CC="aarch64-linux-gnu-gcc"
export AR="aarch64-linux-gnu-ar"

LLVM_CPP_FLAGS="-DLLVM_NATIVE_TARGETINFO=LLVMInitializeAArch64TargetInfo -DLLVM_NATIVE_TARGET=LLVMInitializeAArch64Target -DLLVM_NATIVE_TARGETMC=LLVMInitializeAArch64TargetMC -DLLVM_NATIVE_ASMPRINTER=LLVMInitializeAArch64AsmPrinter -DLLVM_NATIVE_ASMPARSER=LLVMInitializeAArch64AsmParser"

make -C "$PROJECT_ROOT" archive -j"$(nproc)" \
    BUILD=release \
    DIST_ARCH="aarch64" \
    OBJ_ROOT_DIR="$PWD/build/obj" \
    DEP_ROOT_DIR="$PWD/build/dep" \
    OUT_DIR="$PWD/out" \
    \
    USE_LIB_BACKTRACE=\
    LLVM_CFLAGS="" \
    CFLAGS="-O3 -DNDEBUG -std=c11 -Wall -Wextra -I/usr/aarch64-linux-gnu/include -Iinclude $LLVM_CPP_FLAGS" \
    LLVM_LDFLAGS="-Wl,--start-group /usr/aarch64-linux-gnu/lib/libLLVM*.a -Wl,--end-group -lrt -ldl -lm -lstdc++"
