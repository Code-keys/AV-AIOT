# 交叉编译原理与 MacOS实践
## 交叉编译原理
正常编译过程:<br>
使用本机器的编译器，将源代码编译链接成为一个可以在本机器上运行的程序，这就是正常的编译过程，也称为 Native Compilation 本机编译<br>
<br>
交叉编译:<br>
就是在一个平台（如PC）上生成另外一个平台（Android、iOS或者其他 嵌入式设备）的可执行代码<br>
<br>
为什么要用交叉编译<br>
一是因为计算能力的问题，还有一个重要的原因就是编译工具以及整个编译 过程异常繁琐，所以在这种情况下，直接在ARM平台下进行本机编译几乎是不可能的。而具有更加强劲的计算能力与更大存储空间的PC才是理想的选择<br>
<br>
编译工具:<br>
CC：编译器，对C源文件进行编译处理，生成汇编文件<br>
AS：将汇编文件生成目标文件（汇编文件使用的是指令助记符， AS将它翻译成机器码<br>
AR：打包器，用于库操作，可以通过该工具从一个库中删除或者增加目标代码模块<br>
LD：链接器，为前面生成的目标代码分配地址空间，将多个目标文件链接成一个库或者是可执行文件<br>
GDB：调试工具，可以对运行过程中的程序进行代码调试工作<br>
STRIP：以最终生成的可执行文件或者库文件作为输入，然后消除掉其中的源码<br>
NM：查看静态库文件中的符号表<br>
Objdump：查看静态库或者动态库的方法签名<br>
<br>
Android CPU架构<br>
mips / mips64：极少用于手机可以忽略<br>
x86 / x86_64: x86 架构的手机都会包含由 Intel 提供的称为 Houdini 的指令集动态转码工具，实现对 arm .so的兼容，再考虑 x86 1% 以下的市场占有率，x86 相关的两个 .so 也是可以忽略的<br>
armeabi: ARM v5 这是相当老旧的一个版本，缺少对浮点数计算的硬件支持，在需要大量计算时有性能瓶颈<br>
armeabi-v7a: ARM v7 目前主流版本<br>
arm64-v8a: 64位支持<br>
#
## ACC Macos 下 N20 ~ N23 编译
``` bash 
#!/bin/bash 

export API=28 
export NDK=/Users/cxu/Library/Android/sdk/ndk/21.4.7075529 
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
export API=21

MYPATH=`pwd`

function build_aac
{
./configure \
    --prefix=$PREFIX \
    --disable-static \
    --disable-shared \
    --host=$my_host \

make clean
make -j8
make install
}

#arm64-v8a
PREFIX=$MYPATH/android/arm64-v8a
my_host=aarch64-linux-android
export TARGET=aarch64-linux-android
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
build_aac

#armeabi-v7a
PREFIX=$MYPATH/android/armeabi-v7a
my_host=armv7a-linux-android
export TARGET=armv7a-linux-androideabi
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
build_aac
echo ">>>>>>>>> build done <<<<<<<<"    
# merge to a file
# ar.exe r libALL.a lib1.a lib2.a lib3.a
```
``` bash 
libSBRdec/src/lpp_tran.cpp:122:10: fatal error: 'log/log.h' file not found
#include "log/log.h"
         ^~~~~~~~~~~
1 error generated.
```
解决办法：这是引用android的日志导致的，直接删除include 及相关代码( android_errorWriteLog 函数 )就可以了

#

## Lame Macos 下 N20 ~ N23 编译
``` bash
#!/bin/bash

export NDK=/Users/ruisong/Documents/android-c/ndk/android-ndk-r21e
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
export API=21

MYPATH=`pwd`

function build_lame
{
./configure \
    --prefix=$PREFIX \
		--disable-shared \
		--disable-frontend \
    --host=$my_host \

make clean
make -j8
make install
}

#arm64-v8a
PREFIX=$MYPATH/android/arm64-v8a
my_host=aarch64-linux-android
export TARGET=aarch64-linux-android
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
build_lame

#armeabi-v7a
PREFIX=$MYPATH/android/armeabi-v7a
my_host=armv7a-linux-android
export TARGET=armv7a-linux-androideabi
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
build_lame
```

## X264/265 Macos 下 N20 ~ N23 编译
``` bash
#!/bin/bash

export NDK=/Users/ruisong/Documents/android-c/ndk/android-ndk-r21e
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

#armeabi-v7a
PREFIX=./android/armeabi-v7a
my_host=armv7a-linux-android
export TARGET=armv7a-linux-androideabi
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
build_x264
```

## FFmpeg Macos 下 N20 ~ N23 编译
``` bash
#!/bin/bash
API=28
export NDK=/Users/ruisong/Documents/android-c/ndk/android-ndk-r21e
export PREBUILD=$NDK/toolchains/llvm/prebuilt

function build_so 
{
    make clean
     ./configure \
		--prefix=$PREFIX \
		--cc=$CC \
		--nm=$NM \
		--ar=$AR \
		--arch=arm \
		--enable-small \
		--disable-programs \
		--disable-avdevice \
		--disable-encoders \
		--disable-muxers \
		--disable-filters \
		--cross-prefix=$CROSS_PREFIX \
		--target-os=android \
		--enable-shared \
		--disable-static \
		--enable-cross-compile
    make -j4
    make install
}

#armeabi-v7a
export CPU=armv7a
export CROSS_PREFIX=$TOOLCHAIN/bin/$CPU-linux-android-
export CC=$PREBUILD/darwin-x86_64/bin/$CPU-linux-androideabi$API-clang
export NM=$CROSS_PREFIXnm
export AR=$CROSS_PREFIXar
export PREFIX=./android/$CPU
function build_so 

#armeabi- v8a
export CPU=aarch64
# 同 上 
```

 
##  垃  圾  箱
``` bash 
export PREBUILT=$TOOLCHAIN
export PLATFORM=$NDK_ROOT/platforms/android-28/arch-arm64
export PATH=$PATH:$PREBUILT/bin:$PLATFORM/usr/include:
export AS=$PREBUILT/lib/gcc/arm-linux-androideabi/4.9.x/../../../../arm-linux-androideabi/bin/as
export LD=$PREBUILT/lib/gcc/arm-linux-androideabi/4.9.x/../../../../arm-linux-androideabi/bin/ld
export NM=$PREBUILT/lib/gcc/arm-linux-androideabi/4.9.x/../../../../arm-linux-androideabi/bin/nm
export STRIP=$PREBUILT/lib/gcc/arm-linux-androideabi/4.9.x/../../../../arm-linux-androideabi/bin/strip 
export AR=$PREBUILT/lib/gcc/arm-linux-androideabi/4.9.x/../../../../arm-linux-androideabi/bin/ar
export CXX="${PREBUILT}/bin/aarch64-linux-androideabi28-clang++ --sysroot=${SYS_ROOT}" 
export CC ="${PREBUILT}/bin/aarch64-linux-androideabi28-clang --sysroot=${SYS_ROOT} -march=armv8-a"
export RANLIB=$PREBUILT/bin/arm-linux-androideabi-ranlib

export LDFLAGS="-L$PLATFORM/usr/lib -L$PREBUILT/aarch64-linux-android/lib -march=armv8-a" #arm64-v8a
export CFLAGS="-I$PLATFORM/usr/include -march=armv8-a -mfloat-abi=softfp -mfpu=vfp -ffast-math -O2"
export CPPFLAGS="$CFLAGS"
export CFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="$LDFLAGS"
```