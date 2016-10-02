#!/bin/bash

platform=`uname`

if [[ "$platform" == 'Linux' ]]; then
  export ANDROID_NDK_ROOT=/home/$USER/android-ndk-r12b
  export ANDROID_SDK_ROOT=/home/$USER/android-sdk-linux
  export ANDROID_QT_ROOT=/home/$USER/Qt/5.7/android_armv7
elif [[ "$platform" == 'Darwin' ]]; then
  export ANDROID_NDK_ROOT=/Users/$USER/android-ndk-r12b
  export ANDROID_SDK_ROOT=/Users/$USER/android-sdk-macosx
  export ANDROID_QT_ROOT=/Users/$USER/Qt/5.7/android_armv7
fi

$ANDROID_QT_ROOT/bin/qmake

make install INSTALL_ROOT=../android-build/
cp build.properties ../android-build

mkdir -p ../android-build
$ANDROID_QT_ROOT/bin/androiddeployqt --input "android-deployment-settings.json" --output ../android-build/ --sign /Users/bogdan/keystore/ cristeab --storepass mypass

#rename APK using the version number read from file
FILE="../android-build/bin/QtApp-release.apk"
if [ -f $FILE ]; then
  VER_NO=$(cat "version")
  cp $FILE "MarketProfileForex_v$VER_NO.apk"
fi

