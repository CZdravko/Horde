/*
 * Java_org_cocos2dx_chicago_Chicago.cpp
 *
 *  Created on: 15. okt. 2013
 *      Author: MKulis
 */
#include "cocos2d.h"
#include "CCDirector.h"
#include <jni.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  \
											 "Chicago", \
											 __VA_ARGS__))

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG,  \
											 "Chicago", \
											 __VA_ARGS__))

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,  \
											 "Chicago", \
											 __VA_ARGS__))


using namespace cocos2d;

extern "C" {
JNIEXPORT void JNICALL Java_org_cocos2dx_chicago_Chicago_createAssetManager(
		JNIEnv* env, jobject obj, jobject assetManager) {

	LOGI("SETTING A GLOBAL ASSET MANAGER POINTER");

	AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

	// Store the assest manager for future use.
	CCDirector::setAssetManager(mgr);
}
}

