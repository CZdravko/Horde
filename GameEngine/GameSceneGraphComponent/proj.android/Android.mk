LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gamescenegraph_static

LOCAL_MODULE_FILENAME := libgamescenegraph

LOCAL_SRC_FILES := \
../GameEngine_SceneGraph.cpp \
../main.cpp \
../SceneGrapgPlugin.cpp \
../SceneGraph.cpp \
../SceneGraphComponent.cpp


LOCAL_EXPORT_C_INCLUDES :=  $(LOCAL_PATH)/../../Common

LOCAL_C_INCLUDES :=   $(LOCAL_PATH)/../../Common

LOCAL_LDLIBS := -llog \
                -lz

LOCAL_EXPORT_LDLIBS := -llog \
                       -lz

LOCAL_CFLAGS := -Wno-psabi
LOCAL_EXPORT_CFLAGS := -Wno-psabi

include $(BUILD_STATIC_LIBRARY)
