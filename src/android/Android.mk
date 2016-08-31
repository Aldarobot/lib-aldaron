LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL_image
SDL_MIXER_PATH := ../SDL_mixer
SDL_NET_PATH := ../SDL_net
LIB_ZIP_PATH := ../libzip
CLUMP_PATH := ../clump
NATIVE_PATH := ../native_app_glue

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SDL_IMAGE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SDL_IMAGE_PATH)/external/jpeg-9
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SDL_MIXER_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SDL_NET_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(LIB_ZIP_PATH)/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLUMP_PATH)
LOCAL_C_INCLUDES += $(shell find -L $(LOCAL_PATH)/src/ -type d)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(NATIVE_PATH)

# Add your application source files here...
LOCAL_SRC_FILES := $(NATIVE_PATH)/android_native_app_glue.c
LOCAL_SRC_FILES += $(subst $(LOCAL_PATH)/,, \
	$(shell find -L $(LOCAL_PATH)/src/ -type f -name '*.c'))

LOCAL_SHARED_LIBRARIES := SDL2 clump zip SDL2_image SDL2_net SDL2_mixer

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -landroid -lEGL

# Debuging
LOCAL_CFLAGS := -DJL_DEBUG

include $(BUILD_SHARED_LIBRARY)
