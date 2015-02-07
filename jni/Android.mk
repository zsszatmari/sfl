# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE:=libavcodec
LOCAL_SRC_FILES:=../../ffmpeg-2.4./libandroid/arm/lib/libavcodec.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libavutil
LOCAL_SRC_FILES:=../../ffmpeg-2.4./libandroid/arm/lib/libavutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libavformat
LOCAL_SRC_FILES:=../../ffmpeg-2.4./libandroid/arm/lib/libavformat.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libavresample
LOCAL_SRC_FILES:=../../ffmpeg-2.4./libandroid/arm/lib/libavresample.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libmpg123
LOCAL_SRC_FILES:=../../mpg123-1.19.0/libandroid/armv7/lib/libmpg123.so
include $(PREBUILT_SHARED_LIBRARY)

#TODO: add mpg123, ssl, crypto

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES := rtti exceptions
#needed to include libav, a simple define is not enough for whatever reason
LOCAL_CFLAGS=-D__STDC_CONSTANT_MACROS 
LOCAL_LDLIBS=-latomic -lavcodec -lavutil -lavformat -lavresample -L../libav-9.11/libandroid/arm/lib -lssl -lcrypto -Llib/android -lmpg123 -L../mpg123-1.19.0/libandroid/armv7/lib
LOCAL_C_INCLUDES = core core/Cast core/jsoncpp core/public core/public/Base core/public/Gui core/Base core/google core/Base core/Base/leveldb/include include ../clientdb ../clientdb/sqlite core/vorbis

LOCAL_MODULE    := core	   
#LOCAL_SRC_FILES := ../core/Socket.cpp

LOCAL_PATH=.
LOCAL_SRC_FILES = $(wildcard core/*.cpp)
LOCAL_SRC_FILES += $(wildcard core/*.c)
LOCAL_SRC_FILES += $(wildcard core/Cast/*.cpp)
LOCAL_SRC_FILES += $(wildcard core/Cast/*.cc)
LOCAL_SRC_FILES += $(wildcard core/Base/*.cpp)
LOCAL_SRC_FILES += $(wildcard core/Gui/*.cpp)
LOCAL_SRC_FILES += $(wildcard core/jsoncpp/*.cpp)
LOCAL_SRC_FILES += $(wildcard external/clientdb/*.cpp)
LOCAL_SRC_FILES += $(wildcard external/clientdb/sqlite/*.c)
LOCAL_SRC_FILES += $(wildcard core/boost/system/src/*.cpp)
LOCAL_SRC_FILES += $(wildcard core/ogg/*.c)
LOCAL_SRC_FILES += $(wildcard core/vorbis/*.c)
LOCAL_SRC_FILES += $(wildcard core/google/protobuf/*.cc)
LOCAL_SRC_FILES += $(wildcard core/google/protobuf/io/*.cc)
LOCAL_SRC_FILES += $(wildcard core/google/protobuf/stubs/*.cc)


#$(info $(LOCAL_SRC_FILES))


include $(BUILD_SHARED_LIBRARY)

