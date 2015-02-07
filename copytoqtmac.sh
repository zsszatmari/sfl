#cmake . -DCMAKE_BUILD_TYPE=RELEASE -DOSX_VERSION=0
#cmake . -DCMAKE_BUILD_TYPE=DEBUG -DOSX_VERSION=0

mkdir -p ./platforms/qt/lib/macosx
cp libcore.a ./platforms/qt/lib/macosx/libcore-qt.a

rsync -av core/public/ ./platforms/qt/include/core/

rm -r ./platforms/qt/lib/assets
mkdir ./platforms/qt/lib/assets
cp core/assets/* ./platforms/qt/lib/assets

cp core/assets/desktop/* ./platforms/qt/lib/assets