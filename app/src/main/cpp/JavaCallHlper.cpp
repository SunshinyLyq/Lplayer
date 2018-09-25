//
// create by 李雨晴 on 2018/9/8.
//


#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm=vm;
    //如果是在主线程，则回调
    this->env=env;
    //一旦涉及到jobject 跨线程，跨方法都要写成全局的，因为怕被释放了,因为不是全局引用，出方法就会被释放
    this->instance=env->NewGlobalRef(instance);

    jclass  clazz=env->GetObjectClass(instance);
    onErrorId=env->GetMethodID(clazz,"onError","(I)V");
    onPrepareId=env->GetMethodID(clazz,"onPrepare","()V");

}


JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
}

void JavaCallHelper::onPrepare(int thread) {

    //判断是否是主线程
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onPrepareId);
    } else{
        //子线程
        JNIEnv *env1;
        //获得当前线程的env
        vm->AttachCurrentThread(&env1,0);
        env1->CallVoidMethod(instance,onPrepareId);

        vm->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int thread, int errorCode) {
    if (thread == THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorId,errorCode);
    }else{
        JNIEnv *env1;
        vm->AttachCurrentThread(&env1,0);
        env1->CallVoidMethod(instance,onErrorId,errorCode);
        vm->DetachCurrentThread();
    }

}