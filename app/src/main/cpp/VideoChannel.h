//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_VIDEOCHANNEL_H
#define PROJECTS_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C"{
#include <libswscale/swscale.h>
};



typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id,AVCodecContext *avCodecContext,int fps,AVRational time_base);

    ~VideoChannel();

    void play();

    void stop();

    void decode();

    void render();

    void setRenderFrameCallback(RenderFrameCallback callback);
    void setAudioChannel(AudioChannel *audioChannel);

private:
    pthread_t  pid_decode;
    pthread_t  pid_render;
    SwsContext *swsContext=0;
    RenderFrameCallback callback;
    int fps;
    AudioChannel *audioChannel = 0;


};
#endif //PROJECTS_VIDEOCHANNEL_H
