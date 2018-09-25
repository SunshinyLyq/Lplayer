//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_MACRO_H
#define PROJECTS_MACRO_H

#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"FFMPEG",__VA_ARGS__)

//宏定义子线程，主线程

#define THREAD_MAIN 1
#define THREAD_CHILD 2

#define DELETE(obj)  if(obj) {delete obj;obj=0;}

//错误代码
//打不开视频
#define FFMPEG_CAN_NOT_OPEN_URL 1
//找不到流媒体
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
//找不到解码器
#define FFMPEG_FIND_DECODER_FAIL 3
//无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
//根据流信息 配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
//打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL 7
//没有音视频
#define FFMPEG_NOMEDIA 8


#endif //PROJECTS_MACRO_H
