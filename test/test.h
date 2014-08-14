#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <jni.h>
#include "jni-base.h"

jint jniWriteString(JNIEnv *env, jobject thiz, jstring fileName, jstring strData);

static char *TestClassPath = "test/Test";

static JNINativeMethod functionMethods[] = {
	{"jniWriteString", "(Ljava/lang/String;Ljava/lang/String;)I", (void*)jniWriteString}
};

#endif
