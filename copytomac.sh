#cmake . -DCMAKE_BUILD_TYPE=RELEASE -DOSX_VERSION=1
#cmake . -DCMAKE_BUILD_TYPE=DEBUG -DOSX_VERSION=1

mkdir -p ./platforms/mac/lib/macosx
cp libcore.a ./platforms/mac/lib/x64/libcore.a

rsync -av core/public/ ./platforms/mac/include/core/
#rm -r ../G-Ear-v2/include/core
#cp -r core/public ../G-Ear-v2/include/core

rm -r ./platforms/mac/lib/assets
mkdir ./platforms/mac/lib/assets
cp core/assets/* ./platforms/mac/lib/assets

cp core/assets/desktop/* ./platforms/mac/lib/assets
