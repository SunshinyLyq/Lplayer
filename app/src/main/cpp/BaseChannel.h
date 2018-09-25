//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_BASECHANNEL_H
#define PROJECTS_BASECHANNEL_H

#include "SafeQueue.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *avCodecContext,AVRational time_base) : id(id),avCodecContext(avCodecContext),time_base(time_base){

        packets.setReleaseCallback(releaseAvPacket);
        frames.setReleaseCallback(releaseAvFrame);

    }//声明的时候并且实现

    virtual ~BaseChannel() {

        packets.clear();
        frames.clear();
    }//必须设置为虚函数，因为需要调用子类的析构函数

    static void releaseAvPacket(AVPacket** packet) {

        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }

    }

    static void releaseAvFrame(AVFrame** frame){
        if (frame){
            av_frame_free(frame);
            *frame=0;
        }
    }

    //纯虚函数，相当于抽象方法
    virtual void play()=0;

    virtual void stop() = 0;

    bool isPlaying;
    int id;
    AVCodecContext *avCodecContext;

    //解码数据包
    SafeQueue<AVPacket *> packets;

    //编码数据包
    SafeQueue<AVFrame *> frames;

    AVRational time_base;

public:

    double clock;
};

#endif //PROJECTS_BASECHANNEL_H
