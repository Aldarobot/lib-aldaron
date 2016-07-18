LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := clump

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_CFLAGS := -std=c99

LOCAL_SRC_FILES += $(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.c) \
)

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
