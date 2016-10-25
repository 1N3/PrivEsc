LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := rootsh
LOCAL_SRC_FILES := rootsh.c
LOCAL_PREBUILT_OBJ_FILES := own.o
LOCAL_STATIC_LIBRARIES := libc
LOCAL_MODULE_PATH := $(LOCAL_PATH)

# big hack.
TARGET_GLOBAL_LD_DIRS := $(TARGET_GLOBAL_LD_DIRS) -Wl,-T,$(LOCAL_PATH)/armelf.x,--no-gc-sections

$(LOCAL_PATH)/own.o: $(LOCAL_PATH)/own.c
	PATH=$(CURDIR)/prebuilt/$(HOST_PREBUILT_TAG)/toolchain/arm-eabi-4.2.1/bin:$(PATH) make -C $(CURDIR)/kernel ARCH=arm KBUILD_VERBOSE=$(SHOW_COMMANDS) CROSS_COMPILE=arm-eabi- M=$(LOCAL_PATH) modules

include $(BUILD_EXECUTABLE)

#################################################

include $(CLEAR_VARS)

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := asroot
LOCAL_SRC_FILES := asroot.c
LOCAL_PREBUILT_OBJ_FILES := own.o
LOCAL_STATIC_LIBRARIES := libc
LOCAL_MODULE_PATH := $(LOCAL_PATH)

# big hack.
TARGET_GLOBAL_LD_DIRS := $(TARGET_GLOBAL_LD_DIRS) -Wl,-T,$(LOCAL_PATH)/armelf.x,--no-gc-sections

$(LOCAL_PATH)/own.o: $(LOCAL_PATH)/own.c
	PATH=$(CURDIR)/prebuilt/$(HOST_PREBUILT_TAG)/toolchain/arm-eabi-4.2.1/bin:$(PATH) make -C $(CURDIR)/kernel ARCH=arm KBUILD_VERBOSE=$(SHOW_COMMANDS) CROSS_COMPILE=arm-eabi- M=$(LOCAL_PATH) modules

include $(BUILD_EXECUTABLE)
