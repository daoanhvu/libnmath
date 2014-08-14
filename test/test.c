#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jni.h>
#include "test.h"

jint jniWriteString(JNIEnv *env, jobject thiz, jstring fileName, jstring strData) {
	jint i, result = 0;
	
	FILE *f;

	const char* file_name = (*env)->GetStringUTFChars(env, fileName, NULL);
	int fileNameLength = (*env)->GetStringUTFLength(env, fileName);
	const char* data = (*env)->GetStringUTFChars(env, strData, NULL);
	int dataLength = (*env)->GetStringUTFLength(env, strData);

	f = fopen(file_name, "wb");
	if(f != NULL) {
		result = fwrite(data, 1, dataLength, f);
		fclose(f);
	}

	(*env)->ReleaseStringUTFChars(env, fileName, file_name);	
	(*env)->ReleaseStringUTFChars(env, strData, data);
	

	return result;
}
