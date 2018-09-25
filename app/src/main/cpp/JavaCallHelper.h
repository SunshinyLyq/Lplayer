//
// create by 李雨晴 on 2018/9/8.
//

#ifndef PROJECTS_JAVACALLHELPER_H
#define PROJECTS_JAVACALLHELPER_H

// 接口就是调用Java的方法


#include <jni.h>

class JavaCallHelper{

public:
    JavaCallHelper(JavaVM *vm,JNIEnv *env,jobject instance);
    ~JavaCallHelper();

    //回调Java
    void onError(int thread, int errorCode);
    void onPrepare(int thread);

private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;

    jmethodID onPrepareId;
    jmethodID onErrorId;

};

#endif //PROJECTS_JAVACALLHELPER_H
