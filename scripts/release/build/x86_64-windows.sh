#!/usr/bin/env bash
SCRIPT_DIR="$(dirname -- "$(realpath -- "${BASH_SOURCE[0]}")")"
cd "$("$(dirname "$0")"/directory.sh "x86_64-windows")" || exit

TOOLCHAIN="/opt/cross/x86_64-w64-mingw32"
LLVMPATH="/opt/cross/llvm/x86_64-w64-mingw32"

export CC="$TOOLCHAIN/bin/x86_64-w64-mingw32-gcc"
export AR="$TOOLCHAIN/bin/x86_64-w64-mingw32-ar"

LLVM_FLAGS="$(sed "s|@LLVMPATH@|$LLVMPATH|g" "$SCRIPT_DIR/win-llvm-flags.txt")"

LLVM_CPP_FLAGS="-DLLVM_NATIVE_TARGETINFO=LLVMInitializeX86TargetInfo -DLLVM_NATIVE_TARGET=LLVMInitializeX86Target -DLLVM_NATIVE_TARGETMC=LLVMInitializeX86TargetMC -DLLVM_NATIVE_ASMPRINTER=LLVMInitializeX86AsmPrinter -DLLVM_NATIVE_ASMPARSER=LLVMInitializeX86AsmParser"

make -C "$PROJECT_ROOT" archive -j"$(nproc)" \
    BUILD=release \
    DIST_OS="windows" \
    DIST_ARCH="x86_64" \
    EXE_EXT=".exe" \
    SHARED_EXT=".dll" \
    STATIC_EXT=".a" \
    OBJ_ROOT_DIR="$PWD/build/obj" \
    DEP_ROOT_DIR="$PWD/build/dep" \
    OUT_DIR="$PWD/out" \
    HAS_LLVM="yes" \
    LLVM_LDFLAGS="-Wl,--start-group $LLVM_FLAGS -Wl,--end-group -lstdc++ -lversion -luuid -lole32 -lpsapi -lshell32 -lz -lzstd -lws2_32 -lbcrypt" \
    USE_LIB_BACKTRACE=\
    \
    LLVM_CFLAGS="" \
    CFLAGS="-O3 -DNDEBUG -std=c11 -Wall -Wextra -I$LLVMPATH/include -Iinclude $LLVM_CPP_FLAGS" \
    EXTRA_LDFLAGS="-Wl,--start-group \
        $PWD/out/lib/libelc.a $PWD/out/lib/libelash.a $LLVM_FLAGS -Wl,--end-group \
        -lstdc++ -lversion -luuid -lole32 -lpsapi -lshell32 -lz -lzstd -lws2_32 -lbcrypt \
        -static -static-libgcc -static-libstdc++"


