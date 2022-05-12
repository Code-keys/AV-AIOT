#!/bin/bash 
 
export NDK=/Users/cxu/Library/Android/sdk/ndk/21.4.7075529 
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
export API=28


function build_x264
{
./configure \
    --prefix=$PREFIX \
	--disable-cli \
    --enable-static \
    --enable-pic \
    --host=$my_host \
	--cross-prefix=$CROSS_PREFIX \
    --sysroot=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/sysroot \

make clean
make -j8
make install
}

#arm64-v8a
PREFIX=./android/arm64-v8a
my_host=aarch64-linux-android
export TARGET=aarch64-linux-android
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
build_x264 