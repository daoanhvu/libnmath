#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <assert.h>
#include "test.h"

#ifndef NELEM
#define NELEM(x) ((int)sizeof(x)/sizeof((x)[0]))
#endif

static int register_native_method(JNIEnv *env, const char *clsName, JNINativeMethod *methods, int numOfMethod) {
	jclass cls;
	cls = (*env)->FindClass(env, clsName);

	if(cls == NULL){
		return JNI_FALSE;
	}

	if( (*env)->RegisterNatives(env, cls, methods, numOfMethod) < 0 ) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

/**
 * JNI Callback functions
 * */
jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) {
	JNIEnv *env;
	jint result = -1;
	int isRegMethoOk;

	printf("Chay den day roi 1 \n");

	if( (*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK ) {
		printf("Cannot get Env! \n");
		return result;
	}

	printf("Chay den day roi 2 \n");
	
	isRegMethoOk = register_native_method(env, TestClassPath, functionMethods, 1);
	if(isRegMethoOk < 0){
		printf("Dang ky methods KHONG thang cong \n");
		return -1;
	}

	printf("Dang ky methods thanh cong \n");

	result = JNI_VERSION_1_6;
	return result;
}

/*
void JNI_OnUnload(JavaVM *vm, void *reserved){
}
*/
