# MarektProfileForexCFD

A Chart Application which is able to read forex and cfd data delivered by a broker and display that data as market profile chart.

# Development environment setup

- install Android stand-alone SDK tools. Once the tools installed you need to install at least one Android platform. Go to \<android SDK root\>/tools and start "Android SDK manager"

    ./android sdk

then install all required packages.

- install Android NDK

- generate keystore for APK signing

    keytool -genkey -v -keystore my-key.keystore -alias alias_name -keyalg RSA -keysize 2048 -validity 10000

replace "my-key" and "alias_name" by more specific values related to your project. A file "my-key.keystore" should be generated.

- edit "build.properties" file from the root source folder of your application and set the values used in previous step for keystore generation

- edit "build-android.sh" script and make sure the the paths to Qt, Android SDK and NDK are correct (see steps above)

# APK generation

- start the APK build process from the root source folder with

    ./build-android.sh

At the end of a successful build a signed APK is generated.
