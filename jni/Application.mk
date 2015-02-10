APP_PLATFORM := android-14
# gnustl have problems with atomics, but qt is compiled with libstdc++
APP_STL := gnustl_shared
#NDK_TOOLCHAIN_VERSION := 4.8
#APP_STL := c++_shared
# gcc does not know auto lambda parameter types, clang may know with a special flag hich is disabled by default for libc++, toolchain's Android.mk must be modified (search for c++11)
NDK_TOOLCHAIN_VERSION := clang