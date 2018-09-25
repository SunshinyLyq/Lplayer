//
// create by 李雨晴 on 2018/9/9.
//

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

#include "VideoChannel.h"
#include "BaseChannel.h"
#include "macro.h"


/**
 * 丢已经解码的图像
 * @param q
 */
void dropAvFrame(queue<AVFrame *> &q) {

    if (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAvFrame(&frame);
        q.pop();
    }

}


VideoChannel::VideoChannel(int id, AVCodecContext *avCodecContext, int fps, AVRational time_base)
        : BaseChannel(id,
                      avCodecContext, time_base) {
    this->fps = fps;

    frames.setSyncHandle(dropAvFrame);
}

VideoChannel::~VideoChannel() {
}


void *task_decode(void *args) {

    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();

    return 0;
}


//渲染线程
void *task_render(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);

    videoChannel->render();
    return 0;
}


//需要从队列中读取包，然后进行解码 播放操作
void VideoChannel::play() {
    isPlaying = 1;

    frames.setWork(1);
    packets.setWork(1);

    //1.解码
    pthread_create(&pid_decode, 0, task_decode, this);

    //播放
    pthread_create(&pid_render, 0, task_render, this);
}

//解码,循环解码
void VideoChannel::decode() {
    AVPacket *packet = 0;

    while (isPlaying) {
        //取出一个数据包
        int ret = packets.pop(packet);
        if (!isPlaying) {//如果按了暂停，就不用再取了
            //释放
//            releaseAvPacket(&packet);
            break;
        }

        if (!ret) {//没取出成功，再试一次
            continue;
        }
        //把包丢给解码器
        //解码器上下文
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(&packet);
        if (ret != 0) {
            break;
        }

        AVFrame *frame = av_frame_alloc();

        //从解码器中读取解码后的数据包
        ret = avcodec_receive_frame(avCodecContext, frame);
        //需要读取更多的数据才能读出一个图像
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {//失败
            break;
        }

        //再开一个线程来播放
        //为了防止阻塞读取数据，图像的延迟会比较高
        //保证流畅度，音视频同步
        frames.push(frame);

    }
    //释放
    releaseAvPacket(&packet);
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}


/**
 * 播放
 * 1.获得可用的画布
 */
void VideoChannel::render() {
    //目标获得：RGBA
    swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, 0, 0, 0);//查看ffmpeg例子 scaling_video.c

    //每个画面刷新间隔时间,单位为秒
    double frame_delays = 1.0 / fps;

    AVFrame *frame = 0;

    //指针数组
    uint8_t *dst_data[4];
    int dst_linesize[4];
    //申请内存
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }

        // src_linesize:每一行存放的字节长度
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data),
                  frame->linesize, 0,
                  avCodecContext->height,
                  dst_data,
                  dst_linesize);


        //获得当前这一个画面播放的相对的时间
        double clock = frame->best_effort_timestamp * av_q2d(time_base);
        //额外的间隔时间
        double extra_delays = frame->repeat_pict / (2 * fps);
        //真实需要的间隔时间
        double delays = extra_delays + frame_delays;

        if (!audioChannel) {
            av_usleep(delays * 1000000);
        } else {
            if (clock == 0) {
                //休眠
                av_usleep(delays * 1000000);
            } else {
                //比较音频与视频
                double audioClock = audioChannel->clock;
                //间隔
                double diff = clock - audioClock;
                if (diff > 0) {
                    //大于0，表示视频比较快
                    av_usleep(delays * 1000000);
                }else if( diff < 0){
                    //音频比较快
                    //如果视频包积压的太多了，直接丢包
                    if (fabs(diff) >= 0.05) {
                        releaseAvFrame(&frame);
                        //丢包
                        frames.sync();
                        continue;
                    }
                }

            }
        }

        //将解码后的数据，回调出去进行播放
        callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        //释放
        releaseAvFrame(&frame);
    }

    //释放
    av_freep(&dst_data[0]);
    releaseAvFrame(&frame);

    isPlaying = 0;
    sws_freeContext(swsContext);
    swsContext = 0;
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}

void VideoChannel::stop() {
    isPlaying = 0;
    frames.setWork(0);
    packets.setWork(0);

    pthread_join(pid_decode,0);
    pthread_join(pid_render,0);
}