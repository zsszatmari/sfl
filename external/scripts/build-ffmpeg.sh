CURRENTPATH=`pwd`

mkdir -p "${CURRENTPATH}/bin"
mkdir -p "${CURRENTPATH}/lib"

if [ "$1" == "ios" ] ; then
	ARCH=armv7
	PLATFORM=iPhoneOS
	SDKVERSION=8.1
	IOSBUILD=1
elif [ "$1" == "ios-sim" ] ; then
	ARCH=i386
	PLATFORM=iPhoneSimulator
	SDKVERSION=8.1
	IOSBUILD=1
elif [ "$1" == "osx" ] ; then
	ARCH=x86_64
	PLATFORM=osx
elif [ "$1" == "win" ] ; then
	ARCH=i386
	PLATFORM=win
else
	echo "invalid target"
	exit 1
fi


echo building ffmpeg for $PLATFORM

COMMONFLAGS="--disable-everything --disable-programs --disable-lzma --disable-swscale --disable-avdevice --enable-demuxer=mov --enable-decoder=aac --enable-decoder=aac_latm --enable-avresample --enable-filter=volume --enable-filter=equalizer --enable-filter=aresample --enable-filter=aformat --prefix=${CURRENTPATH}/lib/$1"

make distclean 2>/dev/null

if [ "$IOSBUILD" ] ; then

	SDKVERSION=8.1
	DEVELOPER=`xcode-select -print-path`
	export CROSS_BASE="${DEVELOPER}"
	export CROSS_TOP="${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer"
	export CROSS_SDK="${PLATFORM}${SDKVERSION}.sdk"


	export CPP="xcrun --sdk iphoneos cpp"
	export CC="${CROSS_BASE}/usr/bin/gcc -arch $ARCH"
	export LD="${CROSS_BASE}/usr/bin/ld -arch $ARCH"
	export AS="xcrun --sdk iphoneos as"
	export RANLIB="xcrun --sdk iphoneos ranlib"
	export AR="xcrun --sdk iphoneos ar"
	export NM="xcrun --sdk iphoneos nm"  
	#no libtool!
	export LIBTOOL="xcrun --sdk iphoneos libtool" 

	export SYSROOT="${CROSS_TOP}/SDKs/${CROSS_SDK}"
	echo "system root: $SYSROOT"


	export CFLAGS="-miphoneos-version-min=6.1 -arch $ARCH -isysroot $SYSROOT"
	export LDFLAGS="-ios_version_min 6.1 -syslibroot $SYSROOT -lcrt1.o -lc"

	PLATFORMFLAGS="--disable-shared --enable-static --enable-cross-compile --arch=$ARCH --target-os=darwin --cc=\"$CC\" --ld=ld --disable-asm"
else
	if [ "$1" == "osx" ] ; then
		export CFLAGS="-mmacosx-version-min=10.7"
		export LDFLAGS="-mmacosx-version-min=10.7"
	fi

	PLATFORMFLAGS="--enable-shared --disable-static"
fi


#mkdir -p "${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"
#LOG="${CURRENTPATH}/bin/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/build-openssl-${VERSION}.log"

#--disable-asm is needed on ios... even --disable-inline-asm does not suffice

ALLFLAGS="$COMMONFLAGS $PLATFORMFLAGS"
# 300kb but gives error when parsing header...	
#./configure --disable-everything --disable-programs --enable-demuxer=mpegvideo --enable-decoder=aac --enable-decoder=aac_latm --enable-shared --prefix=${CURRENTPATH}/lib/$ARCH --enable-cross-compile --arch=armel --target-os=linux --cross-prefix=${CROSS_TOP}/bin/arm-linux-gnueabi- --sysroot=$SYSROOT
# very large (60mb!), should parse everything imaginable
#./configure --disable-programs --disable-shared --enable-static --prefix=${CURRENTPATH}/lib/$ARCH --enable-cross-compile --arch=arm --target-os=darwin --cc="$CC" --ld=$"LD"
#smallish (5000kb), working for ac3 audio from youtube

echo ./configure $ALLFLAGS
eval "./configure $ALLFLAGS"
#android equivalent:
#./configure --disable-everything --disable-programs --enable-demuxer=mov --enable-decoder=aac --enable-decoder=aac_latm --enable-shared --disable-static --prefix=${CURRENTPATH}/libandroid/$ARCH --enable-cross-compile --arch=arm --target-os=linux --cc=arm-linux-androideabi-gcc --ld=arm-linux-androideabi-ld


# might not be necessary for ios
# --cross-prefix=${CROSS_TOP}/bin/arm-linux-gnueabi- 


#needed for preprocessor on ARM
if [ "$IOSBUILD" ] ; then
	echo "#undef HAVE_TRUNC" >>config.h
	echo "#undef HAVE_TRUNCF" >>config.h
	echo "#undef HAVE_LRINT" >>config.h
	echo "#undef HAVE_LRINTF" >>config.h
	echo "#undef HAVE_ROUND" >>config.h
	echo "#undef HAVE_ROUNDF" >>config.h
	echo "#undef HAVE_RINTF" >>config.h
	echo "#undef HAVE_RINT" >>config.h
	echo "#undef HAVE_ISINF" >>config.h
	echo "#undef HAVE_ISINFF" >>config.h
	echo "#undef HAVE_CBRTF" >>config.h
	echo "#undef HAVE_ISNAN" >>config.h
	echo "#undef HAVE_ISNANF" >>config.h
	echo "#undef HAVE_CBRT" >>config.h
	echo "#undef HAVE_FMINF" >>config.h
	echo "#undef HAVE_GMTIME_R" >>config.h
	echo "#undef HAVE_LOCALTIME_R" >>config.h


	echo "#define HAVE_TRUNC 1" >>config.h
	echo "#define HAVE_TRUNCF 1" >>config.h
	echo "#define HAVE_LRINT 1" >>config.h
	echo "#define HAVE_LRINTF 1" >>config.h
	echo "#define HAVE_ROUND 1" >>config.h
	echo "#define HAVE_ROUNDF 1" >>config.h
	echo "#define HAVE_RINTF 1" >>config.h
	echo "#define HAVE_RINT 1" >>config.h
	echo "#define HAVE_ISINF 1" >>config.h
	echo "#define HAVE_ISINFF 1" >>config.h
	echo "#define HAVE_CBRTF 1" >>config.h
	echo "#define HAVE_ISNAN 1" >>config.h
	echo "#define HAVE_ISNANF 1" >>config.h
	echo "#define HAVE_CBRT 1" >>config.h
	echo "#define HAVE_FMINF 1" >>config.h
	echo "#define HAVE_GMTIME_R 1" >>config.h
	echo "#define HAVE_LOCALTIME_R 1" >>config.h

	echo "#undef getenv" >>config.h
fi

echo start build 
if [ "$1" == "win" ] ; then
	mingw32-make && mingw32-make install
else
	make && make install
fi

