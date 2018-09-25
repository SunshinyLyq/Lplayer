//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_DNFFMPEG_H
#define PROJECTS_DNFFMPEG_H

#include "JavaCallHelper.h"
#include <pthread.h>
#include "AudioChannel.h"
#include "VideoChannel.h"


extern "C"{
#include <libavformat/avformat.h>
};


class DNFFmpeg{
public:
    DNFFmpeg(JavaCallHelper *javaCallHelper, const char* dataSource);
    ~DNFFmpeg();

    void prepare();
    void _prepare();


    void start();
    void _start();

    void stop();

    void setRenderFrameCallback(RenderFrameCallback callback);

public:
    pthread_t pid;
    pthread_t pid_play;
    pthread_t pid_stop;
    char *dataSource;
    JavaCallHelper *javaCallHelper;
    AVFormatContext *formatContext =0 ;
    AudioChannel *audioChannel =0; //初始化，要不然可能会指向不块不可描述的地址，导致会有默认值
    VideoChannel *videoChannel=0;
    RenderFrameCallback callback;

    bool isPlaying ;




};

#endif //PROJECTS_DNFFMPEG_H
