#cmake . -DCMAKE_BUILD_TYPE=RELEASE -DOSX_VERSION=1
#cmake . -DCMAKE_BUILD_TYPE=DEBUG -DOSX_VERSION=1

mkdir -p ../gear-android/gear/src/main/jni/lib/arm
cp libs/armeabi/*.so ../gear-android/gear/src/main/jni/lib/arm
#mv ../gear-droid/jni/lib/arm/libavcodec.so ../gear-droid/jni/lib/arm/libavcodec.so.54.35.0
#mv ../gear-droid/jni/lib/arm/libavformat.so ../gear-droid/jni/lib/arm/libavformat.so.54.20.3
#mv ../gear-droid/jni/lib/arm/libavutil.so ../gear-droid/jni/lib/arm/libavutil.so.52.3.0
#mv ../gear-droid/jni/lib/arm/libavresample.so ../gear-droid/jni/lib/arm/libavresample.so.1.0.1

mkdir -p ../gear-android/gear/src/main/jni/include/core/
rsync -av core/public/ ../gear-android/gear/src/main/jni/include/core/
#rm -r ../G-Ear-v2/include/core
#cp -r core/public ../G-Ear-v2/include/core

rm -r ../gear-android/gear/src/main/assets
mkdir -p ../gear-android/gear/src/main/assets
cp core/assets/* ../gear-android/gear/src/main/assets

#cp core/assets/desktop/* ../G-Ear-v2/lib/assets
