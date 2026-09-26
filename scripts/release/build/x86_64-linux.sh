#!/usr/bin/env bash
cd "$("$(dirname "$0")"/directory.sh "x86_64-linux")" || exit

TOOLCHAIN="/opt/cross/x86_64-linux-musl"
LLVMPATH="/opt/cross/llvm/x86_64-linux-musl"

TOOLCHAIN_GXX_DIR=$(dirname "$($TOOLCHAIN/bin/x86_64-linux-musl-g++ -print-file-name=libstdc++.a)")
TOOLCHAIN_GCC_DIR=$(dirname "$($TOOLCHAIN/bin/x86_64-linux-musl-gcc -print-file-name=libc.a)")

MUSL_DEPS="$HOME/build/musl-libs"

export CC="$TOOLCHAIN/bin/x86_64-linux-musl-gcc"
export AR="$TOOLCHAIN/bin/x86_64-linux-musl-ar"

export EXTRA_LDFLAGS="-static -L${TOOLCHAIN_GXX_DIR} -L${TOOLCHAIN_GCC_DIR} -L${MUSL_DEPS}/lib"
export EXTRA_CFLAGS="-I${MUSL_DEPS}/include -DELC_SHOW_BACKTRACE=0"

make -C "$PROJECT_ROOT" archive -j"$(nproc)" \
    BUILD=release \
    LLVM_CONFIG="$LLVMPATH/bin/llvm-config" \
    USE_LIB_BACKTRACE=\
    EXTRA_CFLAGS="$EXTRA_CFLAGS" \
    EXTRA_LDFLAGS="$EXTRA_LDFLAGS" \
    OBJ_ROOT_DIR="$PWD/build/obj" \
    DEP_ROOT_DIR="$PWD/build/dep" \
    OUT_DIR="$PWD/out"
