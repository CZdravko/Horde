LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gameengincore_static

LOCAL_MODULE_FILENAME := libgameengincore

LOCAL_SRC_FILES := \
../DynLib.cpp \
../GameComponent.cpp \
../GameComponentRegistry.cpp \
../GameEngine_Events.cpp \
../GameEngine_Lua.cpp \
../GameEngine_Network.cpp \
../GameEngine.cpp \
../GameEntity.cpp \
../GameLog.cpp \
../GameModules.cpp \
../GameNetworkManager.cpp \
../GamePlugInManager.cpp \
../GameState.cpp \
../GameWorld.cpp \
../PerformanceTimer.cpp \
../TimingManager.cpp


LOCAL_EXPORT_C_INCLUDES :=  $(LOCAL_PATH)/../../Common

LOCAL_C_INCLUDES :=   $(LOCAL_PATH)/../../Common

LOCAL_LDLIBS := -llog \
                -lz

LOCAL_EXPORT_LDLIBS := -llog \
                       -lz

LOCAL_CFLAGS := -Wno-psabi
LOCAL_EXPORT_CFLAGS := -Wno-psabi

include $(BUILD_STATIC_LIBRARY)
