#!/bin/bash
NDK=/Users/cxu/Library/Android/sdk/ndk/21.4.7075529
HOST_TAG=darwin-x86_64
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

ANDROID_LIB_PATH="$(pwd)/android"

export INCLUDE=$NDK_ROOT/sysroot/usr/include/arm-linux-androideabi 

API=28
./autogen.sh
function build_android_arm
{
echo "build for android $CPU"
./configure \
--host=$HOST \
--disable-shared \
--enable-static \
--prefix="$ANDROID_LIB_PATH/$CPU-$API" 

sudo make clean
sudo make -j8
sudo make install
echo "building for android $CPU completed"
}

echo ">>>>>>>>> building libfaac-2.0.2 for android armeabi-v8a <<<<<<<<" 
CPU=armv8-a
HOST=aarch64-linux-android
export AR=$TOOLCHAIN/bin/aarch64-linux-android-ar
export AS=$TOOLCHAIN/bin/aarch64-linux-android-as
export LD=$TOOLCHAIN/bin/aarch64-linux-android-ld
export RANLIB=$TOOLCHAIN/bin/aarch64-linux-android-ranlib
export STRIP=$TOOLCHAIN/bin/aarch64-linux-android-strip
export CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
export CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
export CFLAGS="-Os -fpic $OPTIMIZE_CFLAGS"
export CPPFLAGS="-Os -fpic $OPTIMIZE_CFLAGS" 
build_android_arm 
echo ">>>>>>>>> build done <<<<<<<<"  

echo ">>>>>>>>> building libfaac for android armeabi-v8a <<<<<<<<" 
CPU=armv7-a
HOST=armv7a-linux-android
export AR=$TOOLCHAIN/bin/armv7a-linux-android-ar
export AS=$TOOLCHAIN/bin/armv7a-linux-android-as
export LD=$TOOLCHAIN/bin/armv7a-linux-android-ld
export RANLIB=$TOOLCHAIN/bin/armv7a-linux-android-ranlib
export STRIP=$TOOLCHAIN/bin/armv7a-linux-android-strip
export CC=$TOOLCHAIN/bin/armv7a-linux-android$API-clang
export CXX=$TOOLCHAIN/bin/armv7a-linux-android$API-clang++
export CFLAGS="-Os -fpic $OPTIMIZE_CFLAGS"
export CPPFLAGS="-Os -fpic $OPTIMIZE_CFLAGS" 
build_android_arm 
echo ">>>>>>>>> build done <<<<<<<<"  