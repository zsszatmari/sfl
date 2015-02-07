#cmake . -DCMAKE_BUILD_TYPE=RELEASE -DOSX_VERSION=1
#cmake . -DCMAKE_BUILD_TYPE=DEBUG -DOSX_VERSION=1

mkdir -p ../G-Ear-v2/lib/macosx
cp debug/libcore.a ../G-Ear-v2/lib/x64/libcore.a

rsync -av core/public/ ../G-Ear-v2/include/core/
#rm -r ../G-Ear-v2/include/core
#cp -r core/public ../G-Ear-v2/include/core

rm -r ../G-Ear-v2/lib/assets
mkdir ../G-Ear-v2/lib/assets
cp core/assets/* ../G-Ear-v2/lib/assets

cp core/assets/desktop/* ../G-Ear-v2/lib/assets
