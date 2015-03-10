cd ffmpeg
sh ../scripts/build-ffmpeg.sh ios
sh ../scripts/build-ffmpeg.sh ios-sim
sh ../scripts/build-ffmpeg.sh osx
cd ..

rsync -r ffmpeg/lib/osx/include/ ../include

rm ../platforms/mac/lib/x64/libav*
rm ../platforms/mac/lib/x64/libsw*
for i in $(find ffmpeg/lib/osx/lib -maxdepth 1 ! -type l) ; do cp $i ../platforms/mac/lib/x64 ; done

cp ffmpeg/lib/ios/lib/*.a ../platforms/ios/lib/iphoneos
cp ffmpeg/lib/ios-sim/lib/*.a ../platforms/ios/lib/iphonesimulator


