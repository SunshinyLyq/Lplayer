//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_AUDIOCHANNEL_H
#define PROJECTS_AUDIOCHANNEL_H

#include "BaseChannel.h"

 #include <SLES/OpenSLES.h>
 #include <SLES/OpenSLES_Android.h>

extern "C" {

#include <libswresample/swresample.h>
};


class AudioChannel :public BaseChannel{

public:
    AudioChannel(int id,  AVCodecContext *avCodecContext,AVRational time_base);

    ~AudioChannel();

    void play();

    //解码
    void decode();

    //播放
    void _play();

    //获取pcm数据
    int getPcm();

    void stop();

public:

    uint8_t *data =0;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;

private:
    pthread_t  pid_audio_play;
    pthread_t  pid_audio_decode;

    // engine interfaces
    SLObjectItf engineObject = NULL;//指针类型
    SLEngineItf engineInterface = 0; //初始化目的，是防止出现if（）判断形式

    //混音器
    SLObjectItf  outputMixObject = 0;

    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf  bqPlayerInterface = 0 ;

    //播放回调队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface =0;

    //重采样
    SwrContext *swrContext = 0 ;


};

#endif //PROJECTS_AUDIOCHANNEL_H
