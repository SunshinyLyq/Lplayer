//
// create by 李雨晴 on 2018/9/9.
//
// opensl Es开发流程
/**
 * 1.创建引擎接口对象
 * 2.创建混音器
 * 3.创建播放器
 * 4.设置播放回调函数
 * 5.设置播放状态
 * 6.启动回调函数
 * 7.释放
 */

#include "AudioChannel.h"


void *audio_decode(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();

    return 0;
}

void *audio_play(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->_play();

    return 0;
}

AudioChannel::AudioChannel(int id, AVCodecContext *avCodecContext,AVRational time_base) : BaseChannel(id,
                                                                                 avCodecContext,time_base) {
    //44100个16位 44100 *2
    //双声道
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;

    data = static_cast<uint8_t *>(malloc(out_sample_rate * out_channels * out_samplesize));
    memset(data, 0, out_sample_rate * out_channels * out_samplesize);
}

AudioChannel::~AudioChannel() {
    if (data) {
        free(data);
        data = 0;
    }
}


void AudioChannel::play() {
    //设置为播放状态
    packets.setWork(1);
    frames.setWork(1);

    //swrcontext + out_ch_layout声道数,给双声道 + 采样位 + 输出的采样率
    // + 输入的声道数 + 输入的采样位 + 输入的采样率
    swrContext = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                                    avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate, 0, 0);

    swr_init(swrContext);

    isPlaying=1;
    //解码
    pthread_create(&pid_audio_decode, 0, audio_decode, this);
    //播放
    pthread_create(&pid_audio_play, 0, audio_play, this);

}

//解码
void AudioChannel::decode() {
    AVPacket *packet = 0;

    while (isPlaying) {
        //取出一个数据包，这个是解码前的数据包
        int ret = packets.pop(packet);

        if (!isPlaying) {
            break;
        }
        //取出失败
        if (!ret) {
            continue;
        }

        //把包丢给解码器
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(&packet);

        if (ret != 0) {
            break;
        }

        AVFrame *frame = av_frame_alloc();
        //从解码器中读取，解码后的数据包
        ret = avcodec_receive_frame(avCodecContext, frame);
        //需要更多的数据才能解码，继续
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            break;
        }

        //将解码后的数据，放入队列中
        frames.push(frame);
    }

    releaseAvPacket(&packet);

};

//返回获取的pcm的数据大小
//frames包中是获取的解码后的音频流
int AudioChannel::getPcm() {

    int data_size = 0;
    AVFrame *frame;
    int ret = frames.pop(frame);
    if (!isPlaying) {
        if (ret) {
            releaseAvFrame(&frame);
        }

        return data_size;
    }


    //假设输入了10个数据。swrContext转码器，这一次只处理了8个数据，还剩2个数据没处理，
    //这一次处理需要把上一次没处理完的一起处理，否则容易积压数据，导致栈内存满了，崩了
    int64_t delays = swr_get_delay(swrContext, frame->sample_rate);

    //将nb_sample个数据，由sample_rate采样率转成44100后，返回多少个数据
    //10 个4800 = nb个44100
    //AV_ROUND_UP up往上入,向上取整
    //单位转换
    int64_t max_samples = av_rescale_rnd(delays + frame->nb_samples, out_sample_rate,
                                         frame->sample_rate, AV_ROUND_UP);

    //获取pcm数据，播放器已经规定的播放数据格式是双声道的，44100采样的，16位的数据
    //获取到的数据格式可能跟规定的数据不一致，所以需要重采样
    //重采样
    //2.输出的缓冲区
    //3.输出缓冲区能接受的最大数据量
    //4.输入的数据
    //5.输入的数据个数

    //返回每一个声道的输出数据
    int samples = swr_convert(swrContext, &data, max_samples, (const uint8_t **) (frame->data),
                              frame->nb_samples);

    //字节大小
    //获取 sample 个 2字节（16位）* 声道
    data_size = samples * out_channels * out_samplesize;

    //获取到音频的相对播放时间
    clock=frame->pts * av_q2d(time_base);
    return data_size;

}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    //播放
    //1.获得pcm数据,多少个字节
    int data_size = audioChannel->getPcm();
    if (data_size > 0) {
        //接收16位数据
        (*bq)->Enqueue(bq, audioChannel->data, data_size);
    }

}

//播放
//opensl Es
void AudioChannel::_play() {
    /**
     *  1.创建引擎并获取引擎接口
     */
    //1.1 创建引擎 SLObjectItf engineObject
    SLresult result;

    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    //assert(SL_RESULT_SUCCESS == result);//断言

    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 1.2 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //1.3 获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
                                           &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }


    /**
     * 2.设置混音器
     */

    // 2.1 创建混音器SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                                 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 2.2 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    /**
     * 3.创建播放器
     */

    //3.1 配置输入声音信息
    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    //pcm数据格式
    /**
     * 声音格式：
     * 采样率（采样频率）：1s中，采集多少次声音 单位赫兹
     * 采样位：16位，就是2字节
     * 声道数：单声道，双声道
     * SL_DATAFORMAT_PCM：声音格式为pcm
     * 2:双声道
     * SL_SAMPLINGRATE_44_1：采样率，人能听到的频率
     * SL_PCMSAMPLEFORMAT_FIXED_16：采样位
     * SL_PCMSAMPLEFORMAT_FIXED_16：数据的大小
     * SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：双声道
     *  SL_BYTEORDER_LITTLEENDIAN:小端数据
     *  大小端：数据的存储方式
     *  pcm是小端
     */
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    //指定输入的数据
    //音源（输入）
    SLDataSource slDataSource = {&android_queue, &pcm};


    //3.2配置音轨（输出）
    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    //需要的接口 ，操作队列的接口，如果需要其他接口，也要加进到这个
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //3.3 创建播放器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                          &audioSnk, 1,
                                          ids, req);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);


    /**
     * 4.设置播放回调函数
     */
    //获取播放器队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueueInterface);
    //设置回调
    (*bqPlayerBufferQueueInterface)->RegisterCallback(bqPlayerBufferQueueInterface,
                                                      bqPlayerCallback, this);

    /**
     * 5.设置播放状态
     */
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    /**
     * 6.手动激活这个回调
     */
    bqPlayerCallback(bqPlayerBufferQueueInterface, this);

}


void AudioChannel::stop() {
    isPlaying=0;
    packets.setWork(0);
    frames.setWork(0);

    //等待解码和播放线程结束
    pthread_join(pid_audio_decode,0);
    pthread_join(pid_audio_play,0);

    if (swrContext){
        swr_free(&swrContext);
        swrContext = 0;
    }

    //释放播放器
    if (bqPlayerObject){
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bqPlayerInterface = 0;
        bqPlayerBufferQueueInterface = 0;
    }

    //释放混音器
    if (outputMixObject){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }

    //释放引擎
    if (engineObject){
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineInterface = 0;
    }

}