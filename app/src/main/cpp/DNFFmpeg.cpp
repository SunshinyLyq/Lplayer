//
// create by 李雨晴 on 2018/9/9.
// 专门处理直播
//


#include <cstring>
#include <pthread.h>
#include "DNFFmpeg.h"
#include "macro.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}


void* task_prepare(void* args){
    //进行准备工作，由于准备工作需要访问datasource这个私有变量，

    DNFFmpeg *ffmpeg= static_cast<DNFFmpeg *>(args);
    ffmpeg->_prepare();

    return 0;
}

DNFFmpeg::DNFFmpeg(JavaCallHelper *javaCallHelper, const char *dataSource) {
    this->javaCallHelper=javaCallHelper;
    //strlen字符串长度不包括 \0
    //这里不可以直接使用，防止dataSource指向的内存被释放
    this->dataSource=new char[strlen(dataSource)+1];
    strcpy(this->dataSource,dataSource);

}

void DNFFmpeg::prepare() {
    //由于准备工作较耗时，这个新开一个线程
    pthread_create(&pid,0,task_prepare,this);

}
/**
 * 准备视频的流程：1.初始化网络
 */
void DNFFmpeg::_prepare() {
    //1.初始化网络
    avformat_network_init();

    //AVFromatContext 包含了视频的信息
    //文件路径不对 手机没网
    AVDictionary *options = 0;
    //设置超时时间，微秒，超时时间5s
    av_dict_set(&options,"timeout","5000000",0);
    //2.打开媒体地址
    //avformat_open_input要求传递的是formatContext指针的指针，对外面的formatContext指针会有影响
    // 内部会对它申请内存，所以外面可以不用申请
    //第三个参数 ：指示打开的媒体格式（传null,ffmpeg会自动推导出是什么格式的媒体）
    int ret = avformat_open_input(&formatContext, dataSource, 0, &options);
    av_dict_free(&options);
    if (ret){
        LOGE("打开媒体文件失败 ：%s",av_err2str(ret));
        if (isPlaying){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;

    }

    //3.查找媒体中音视频流（给context里的参数赋值）
    ret = avformat_find_stream_info(formatContext,0);
    if (ret < 0){
        LOGE("查找流失败 ：%s",av_err2str(ret));
        if (isPlaying){
            javaCallHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    //4.解封装一段视频，获取每段流里面的解码器
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //获取流，可能是音频，也可能是视频
        AVStream *stream= formatContext->streams[i];
        //获取流中的参数,包含了解码这段流的各种参数信息
        AVCodecParameters *codepar= stream->codecpar;

        //查找解码器,通过当前流使用的编码方式，查找
        AVCodec *dec = avcodec_find_decoder(codepar->codec_id);
        if (dec == NULL){
            LOGE("查找解码器失败");
            if (isPlaying){
                javaCallHelper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }

        //获得解码器上下文信息
        AVCodecContext *context=avcodec_alloc_context3(dec);
        if (context == NULL){
            LOGE("获取解码器上下文失败：%s",av_err2str(ret));
            if (isPlaying){
                javaCallHelper->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //设置上下文内的一些参数
        ret = avcodec_parameters_to_context(context,codepar);
        if (ret < 0){
            LOGE("设置解码器上下文参数失败：%s",av_err2str(ret));
            if (isPlaying){
                javaCallHelper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }

        //打开解码器
        ret = avcodec_open2(context,dec,0);
        if (ret){
            LOGE("打开解码器失败：%s",av_err2str(ret));
            if (isPlaying){
                javaCallHelper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }

        //单位
        AVRational time_base=stream->time_base;

        if (codepar->codec_type ==AVMEDIA_TYPE_AUDIO){
            //音频
            audioChannel=new AudioChannel(i,context,time_base);

        }else if(codepar->codec_type == AVMEDIA_TYPE_VIDEO){
            //视频
            //帧率 每秒需要显示多少个图像
            AVRational frame_rate = stream->avg_frame_rate;
            int fps=av_q2d(frame_rate);

            videoChannel=new VideoChannel(i,context,fps,time_base);
            videoChannel->setRenderFrameCallback(callback);
        }

    }

    //没有音视频  (很少见)
    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }

    //准备完了，通知Java
    if (isPlaying){
        javaCallHelper->onPrepare(THREAD_CHILD);
    }

}

void* task_play(void *args){

    DNFFmpeg *dnfFmpeg= static_cast<DNFFmpeg *>(args);
    dnfFmpeg->_start();

    return 0;
}


void DNFFmpeg::start() {
    //开始播放视频
    //解码，读取一个AVPackget
    //从网络读取一个数据包，并且进行解码这是一个持续的过程，为了防止阻塞主线程，所以这里重新开一个子线程进行读取数据包
    //需要判断当前是否是正在播放
    isPlaying = 1;//true

    if (videoChannel){
        //队列开始工作
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }

    if (audioChannel){
        //音频工作
        audioChannel->play();
    }

    pthread_create(&pid_play,0,task_play,this);
}

void DNFFmpeg::_start() {

    //1.读取媒体数据包（音视频的数据包）
    int ret;
    while (isPlaying){
        //读取文件的时候没有网络请求，一下子读完，可能导致oom，
        //特别读本地文件的时候，一下子就读完
        if (audioChannel && audioChannel->packets.size() > 100){
            //10ms
            av_usleep(1000 * 10);
            continue;
        }
        if (videoChannel && videoChannel->packets.size() >100){
            av_usleep(1000 * 10);
            continue;
        }

        AVPacket *packet=av_packet_alloc();
        //因为av_read_frame传递的是packet指针，它不会对外面的packet有影响，所以需要申请内存
        //从媒体文件中读取了一帧数据，将这一帧数据放到了AVPacket中
        ret = av_read_frame(formatContext,packet);
        if (ret == 0){
            //成功
            //读取的这一帧数据有可能是音频数据，也有可能是视频数据
            //怎么判断在，这个stream_index,就是和formatContext->nb_streams中的有关，所以需要保存下来
            if (audioChannel && packet->stream_index == audioChannel->id){
                //音频
                audioChannel->packets.push(packet);
            }else if (videoChannel && packet->stream_index == videoChannel->id){
                //视频
                //packet有很多个，需要个队列
                videoChannel->packets.push(packet);
            }
        }else if (ret == AVERROR_EOF){
            //读取完成，但是可能还没有播放完成
            if (audioChannel->packets.empty() && audioChannel->frames.empty()
                && videoChannel->packets.empty() && videoChannel->frames.empty()){
                break;
            }
            //为什么这里要让它继续循环 而不是sleep
            //如果是做直播 ，可以sleep
            //如果要支持点播(播放本地文件） seek 后退
        }else{
            break;
        }
    }

    isPlaying = 0;

    audioChannel->stop();
    videoChannel->stop();

};

void DNFFmpeg::setRenderFrameCallback(RenderFrameCallback callback) {
   this->callback=callback;
}

DNFFmpeg::~DNFFmpeg() {
    DELETE(dataSource);
}

void *aync_stop(void* args){
    DNFFmpeg *ffmpeg= static_cast<DNFFmpeg *>(args);
    //等待prepare结束
    pthread_join(ffmpeg->pid, 0);
    //保证start线程结束
    pthread_join(ffmpeg->pid_play,0);

    DELETE(ffmpeg->videoChannel);
    DELETE(ffmpeg->audioChannel);

    //释放
    if (ffmpeg->formatContext){
        //先关闭读取
        avformat_close_input(&ffmpeg->formatContext);
        avformat_free_context(ffmpeg->formatContext);
        ffmpeg->formatContext = 0;
    }

    DELETE(ffmpeg);

    return 0;
}

void DNFFmpeg::stop() {
    isPlaying = 0;
    javaCallHelper = 0;
    pthread_create(&pid_stop, 0, aync_stop, this);
}