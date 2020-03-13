#!/bin/sh

# ####################################################
# Build script for g2o for iOS
# ####################################################

ARCH=linux
ZIPFILE="$ARCH"_g2o
ZIPFOLDER="build/$ZIPFILE"
BUILD_D=build/"$ARCH"_debug
BUILD_R=build/"$ARCH"_release

clear
echo "Building g2o using the sources in the thirdparty directory"
cd ../g2o

# Make build folder for debug version
mkdir build
rm -rf $BUILD_D
mkdir "$BUILD_D"
cd "$BUILD_D"

echo "====================================================== cmake"
# Run cmake to configure and generate for iosV8 debug
cmake \
-DCMAKE_INSTALL_PREFIX=install \
-DG2O_BUILD_APPS=off \
-DG2O_BUILD_EXAMPLES=off \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_DEBUG_POSTFIX="" \
-DEIGEN3_INCLUDE_DIR=../eigen \
-DG2O_USE_OPENGL=off \
-GXcode \
-DPLATFORM=OS64 \
-DCMAKE_TOOLCHAIN_FILE=../../../prebuild_scripts/ios.toolchain.cmake \
-DENABLE_ARC=false \
../..

cmake --build . --config Debug --target install

cd ../.. # back to opencv

# Make build folder for release version
echo "============================================================"
rm -rf $BUILD_R
mkdir $BUILD_R
cd $BUILD_R

# Run cmake to configure and generate the make files
cmake \
-DCMAKE_INSTALL_PREFIX=install \
-DG2O_BUILD_APPS=off \
-DG2O_BUILD_EXAMPLES=off \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_DEBUG_POSTFIX="" \
-DEIGEN3_INCLUDE_DIR=../eigen \
-DG2O_USE_OPENGL=off \
-GXcode \
-DPLATFORM=OS64 \
-DCMAKE_TOOLCHAIN_FILE=../../../prebuild_scripts/ios.toolchain.cmake \
-DENABLE_ARC=false \
../..

cmake --build . --config Release --target install

cd ../.. # Back to opencv

# Create zip folder for debug and release version
rm -rf $ZIPFOLDER
mkdir $ZIPFOLDER
cp -R $BUILD_R/install/include $ZIPFOLDER/include
cp -R $BUILD_R/install/lib     $ZIPFOLDER/release
cp -R $BUILD_D/install/lib     $ZIPFOLDER/debug
cp LICENSE $ZIPFOLDER
cp README.md $ZIPFOLDER
