cd ffmpeg
sh ../scripts/build-ffmpeg.sh win
cd ..

# skip include, it's already copied on mac
#rsync -r ffmpeg/lib/osx/include/ ../include

rm ../platforms/qt/lib/win/av*
rm ../platforms/qt/lib/win/sw*

ccp ffmpeg/lib/win/bin/*.dll ../platforms/qt/lib/win


