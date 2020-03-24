#include "base.h"

extern int register_jni(JNIEnv *env);

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    int err;

    if((err = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6)) != JNI_OK)
    {
        LOGE("Get Env Error: %d", err);
        return JNI_ERR;
    }
    err = register_jni(env);
    if(err != 0)
    {
        LOGE("Register Jni Error: %d", err);
    }
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    LOGD("on jni unload");
}