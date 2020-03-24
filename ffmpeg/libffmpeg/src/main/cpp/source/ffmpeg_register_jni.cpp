#include "base.h"
#include <android/native_window_jni.h>

#define CLASS_NAME "org/ffmpeg/utils/FFmpegUtils"
#define STREAM_CALLBACK_CLASS_NAME "org/ffmpeg/utils/StreamCallback"

extern "C" {

#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include <libavutil/time.h>

}

int flag = 0;
void log_callback(void *arg1, int arg2, const char *arg3, va_list arg4) {
    // LOGD("log_callback: arg1: %d\narg2: %d\narg3:%s", arg1, arg2, arg3);
}

void nativePrintFFmpegInfo(JNIEnv *env, jclass clz) {
    const char *version_info = av_version_info();
    LOGD("version_info, %s", version_info);
}

// android中无法打印多媒体文件的信息;
jint nativeDumpMediaInfo(JNIEnv *env, jclass clz, jstring file_path) {
    LOGD("start nativeDumpMediaInfo");

    av_log_set_level(AV_LOG_DEBUG);
    // void (*callback)(void*, int, const char*, va_list)
    av_log_set_callback(&log_callback);
    // 1. 注册锁头的解码器, 新版本的ffmpeg已经不需要调用了, 为了兼容ffmpeg版本, 这里也调用一下;
    av_register_all();

    AVFormatContext *av_fc = NULL;
    size_t av_codec_size = 0;
    AVCodec *av_codec = NULL;

    const char *media_path = env->GetStringUTFChars(file_path, NULL);

    int ret = 0;
    // 2. 打开文件, 参数三是多媒体文件的格式, 如果为NULL, 则根据参数二输入文件的后缀来判断文件格式
    ret = avformat_open_input(&av_fc, media_path, NULL, NULL);

    if (ret != 0) {
        ret = -1;
        goto __END;
    }

    ret = avformat_find_stream_info(av_fc, NULL);
    if (ret < 0) {
        ret = -2;
        goto __END;
    }

    av_codec = av_fc->video_codec;
    while (av_codec != NULL) {
        LOGD("av_codec: %s", av_codec->name);

        av_codec = av_codec->next;
    }
    av_codec_size = ARRAY_LENGTH(av_codec);
    for (int i = 0; i < av_codec_size; i++) {
    }
    LOGD("duration: %lld %d", av_fc->duration, av_codec_size);

    __END:
    avformat_close_input(&av_fc);
    env->ReleaseStringUTFChars(file_path, media_path);
    LOGD("end nativeDumpMediaInfo");

    return ret;
}

JNIEXPORT void JNICALL
nativePlayStream(JNIEnv *env, jclass type, jstring videoPath_,
                 jobject surface, jobject stream_callback) {

    const char *video_path = env->GetStringUTFChars(videoPath_, NULL);

    // 1. 注册编码器
    avcodec_register_all();

    avformat_network_init();

    // 2. 分配一个AVFormatContext
    AVFormatContext *av_fc = avformat_alloc_context();

    if (av_fc == NULL) {
        LOGE("avformat_alloc_context failed!");
        return;
    }
    int ret = -1;
    int ret_avformat_open_input = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    AVCodec *codec_decoder = NULL;
    AVPacket *avPacket = NULL;
    AVFrame *frame = NULL;
    AVFrame *renderFrame = NULL;
    AVFrame *callbackFrame = NULL;
    int size = 0;
    int callback_size = 0;
    uint8_t *buffer = NULL;
    uint8_t *callback_buffer = NULL;
    struct SwsContext *swsContext = NULL;
    struct SwsContext *nv21_swsContext = NULL;
    ANativeWindow *nativeWindow = NULL;
    ANativeWindow_Buffer windowBuffer;
    int video_stream_index = -1;
    jclass stream_callback_jclass;
    jmethodID stream_callback_methodid;

    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;
    AVPixelFormat callback_dstFormat = AV_PIX_FMT_NV21;

    // 3. 打开文件流
    ret_avformat_open_input = avformat_open_input(&av_fc, video_path, NULL, NULL);
    if (ret_avformat_open_input != 0) {
        LOGE("open input file failed!");
        goto __END;
    }

    // 4. 检索多媒体流的信息, 其结果将会被赋值在av_fc中
    ret = avformat_find_stream_info(av_fc, NULL);
    if (ret < 0) {
        LOGE("avformat_find_stream_info failed: %d", ret);
        goto __END;
    }

    // 5. 找到视频流的索引

    for (int i = 0; i < av_fc->nb_streams; i++) {
        if (av_fc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        LOGE("no video stream found!");
        goto __END;
    }

    // 6.根据视频的编码ID找到编码器
    avCodecParameters = av_fc->streams[video_stream_index]->codecpar;
    codec_decoder = avcodec_find_decoder(avCodecParameters->codec_id);
    if (codec_decoder == NULL) {
        LOGE("codec decoder not found");
        goto __END;
    }

    // 7.创建一个编解码器上下文
    avCodecContext = avcodec_alloc_context3(codec_decoder);
    if (avCodecContext == NULL) {
        LOGE("alloc avcodec context fail");
        goto __END;
    }

    ret = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    if (ret < 0) {
        LOGE("fill avcode context by media avcodec parameters failed");
        goto __END;
    }

    // 8. 打开解码器
    ret = avcodec_open2(avCodecContext, codec_decoder, NULL);
    if (ret != 0) {
        LOGE("open avcodec failed %d", ret);
        goto __END;
    }



    // 9 分配内存空间windowBuffer
    avPacket = av_packet_alloc(); // 用于保存从多媒体中读取的数据包
    frame = av_frame_alloc();      // 用于保存从编码器读出来的数据帧
    renderFrame = av_frame_alloc();// 用户保存转换成RGBA后的数据帧
    callbackFrame = av_frame_alloc();

    size = av_image_get_buffer_size(dstFormat, avCodecContext->width, avCodecContext->height,
                                    1);
    buffer = (uint8_t *) malloc(size * sizeof(uint8_t *));
    av_image_fill_arrays(renderFrame->data, renderFrame->linesize, buffer, dstFormat,
                         avCodecContext->width, avCodecContext->height, 1);

    callback_size = av_image_get_buffer_size(callback_dstFormat, avCodecContext->width,
                                             avCodecContext->height,
                                             1);
    callback_buffer = (uint8_t *) malloc(callback_size * sizeof(uint8_t *));
    av_image_fill_arrays(callbackFrame->data, callbackFrame->linesize, callback_buffer,
                         callback_dstFormat, avCodecContext->width, avCodecContext->height, 1);

    // 10.创建swsContext, 用于格式转换
    // avCodecContext 默认是 AV_PIX_FMT_YUV420P; 这里的YUV420P是 I420格式的, YYYY UU VV
    LOGD("before swsContext %d %d, %d %d", avCodecContext->width, avCodecContext->height,
         avCodecContext->pix_fmt, AV_PIX_FMT_RGBA);
    swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, dstFormat,
            SWS_BILINEAR,
            NULL, NULL, NULL);

    nv21_swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, callback_dstFormat,
            SWS_BILINEAR,
            NULL, NULL, NULL);


    if (swsContext == NULL || nv21_swsContext == NULL) {
        LOGE("sws_getContext failed");
        goto __END;
    }

    // 11. 初始化ANativeWindow
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == NULL) {
        LOGE("create native window failed");
        goto __END;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow, avCodecContext->width, avCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);

    LOGD("before av_read_frame");
    stream_callback_jclass = env->FindClass(STREAM_CALLBACK_CLASS_NAME);
    stream_callback_methodid = env->GetMethodID(stream_callback_jclass, "onStreamCallback",
                                                "([BII)V");
    flag = 1;
    // 12. 开始读取视频和渲染
    // av_read_frame: 0 if OK, < 0 on error or end of file
    while (av_read_frame(av_fc, avPacket) == 0 && flag) {
        // 如果读取到的数据包为视频
        if (avPacket->stream_index == video_stream_index) {
            // 发送给解码器
            int sendState = avcodec_send_packet(avCodecContext, avPacket);
            if (sendState == 0) {
                int receiveState = avcodec_receive_frame(avCodecContext, frame);
                // LOGD("receive frame %d", receiveState);
                if (receiveState == 0) {
                    sws_scale(swsContext,
                              (uint8_t const *const *) frame->data,
                              frame->linesize,
                              0,
                              avCodecContext->height, renderFrame->data, renderFrame->linesize);

                    sws_scale(nv21_swsContext,
                              (uint8_t const *const *) frame->data,
                              frame->linesize,
                              0,
                              avCodecContext->height, callbackFrame->data, callbackFrame->linesize);

                    jbyteArray array = env->NewByteArray(callback_size);

                    //HERE I GET THE ERROR, I HAVE BEEN TRYING WITH len/2 and WORKS , PROBABLY SOME BYTS ARE GETTING LOST.
                    env->SetByteArrayRegion(array, 0, callback_size,
                                            (jbyte *) (callbackFrame->data[0]));

                    LOGD("after av_read_frame: %d", sizeof(callbackFrame->data[0]));
                    env->CallVoidMethod(stream_callback, stream_callback_methodid, array,
                                        avCodecContext->width, avCodecContext->height);

                    env->DeleteLocalRef(array);
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;
                    uint8_t *src = renderFrame->data[0];

                    int dst_stride = windowBuffer.stride * 4;
                    int src_stride = renderFrame->linesize[0];

                    for (int i = 0; i < avCodecContext->height; i++) {
                        memcpy(dst + i * dst_stride, src + i * src_stride, src_stride);
                    }

                    ANativeWindow_unlockAndPost(nativeWindow);
                } else {
                    LOGW("avcodec receive frame failed %d", receiveState);
                }
            } else {
                LOGW("avcodec send packet failed %d", sendState);
            }
        }
        av_packet_unref(avPacket);
    }

    __END:
    if (ret_avformat_open_input == 0) { avformat_close_input(&av_fc); }
    avformat_free_context(av_fc);
    if (avCodecContext != NULL) { avcodec_free_context(&avCodecContext); }
    if (avPacket != NULL) { av_packet_free(&avPacket); }
    if (frame != NULL) { av_frame_free(&frame); }
    if (renderFrame != NULL) { av_frame_free(&renderFrame); }
    if (nativeWindow != NULL) { ANativeWindow_release(nativeWindow); }
    if (avCodecContext != NULL) { avcodec_close(avCodecContext); }
    if (swsContext != NULL) { sws_freeContext(swsContext); }
    if (buffer != NULL) { free(buffer); }
    if (nv21_swsContext != NULL) { sws_freeContext(nv21_swsContext); }
    if (callback_buffer != NULL) { free(callback_buffer); }
    env->ReleaseStringUTFChars(videoPath_, video_path);
    LOGD("play stream finish ");
}


JNIEXPORT void JNICALL
nativePlayVideo(JNIEnv *env, jclass type, jstring videoPath_,
                jobject surface) {

    const char *video_path = env->GetStringUTFChars(videoPath_, NULL);

    // 1. 注册编码器
    avcodec_register_all();

    // 2. 分配一个AVFormatContext
    AVFormatContext *av_fc = avformat_alloc_context();

    if (av_fc == NULL) {
        LOGE("avformat_alloc_context failed!");
        return;
    }
    int ret = -1;
    int ret_avformat_open_input = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    AVCodec *codec_decoder = NULL;
    AVPacket *avPacket = NULL;
    AVFrame *frame = NULL;
    AVFrame *renderFrame = NULL;
    int size = 0;
    uint8_t *buffer = NULL;
    struct SwsContext *swsContext = NULL;
    ANativeWindow *nativeWindow = NULL;
    ANativeWindow_Buffer windowBuffer;
    int video_stream_index = -1;

    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;

    // 3. 打开文件流
    ret_avformat_open_input = avformat_open_input(&av_fc, video_path, NULL, NULL);
    if (ret_avformat_open_input != 0) {
        LOGE("open input file failed!");
        goto __END;
    }

    // 4. 检索多媒体流的信息, 其结果将会被赋值在av_fc中
    ret = avformat_find_stream_info(av_fc, NULL);
    if (ret < 0) {
        LOGE("avformat_find_stream_info failed: %d", ret);
        goto __END;
    }

    // 5. 找到视频流的索引

    for (int i = 0; i < av_fc->nb_streams; i++) {
        if (av_fc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        LOGE("no video stream found!");
        goto __END;
    }

    // 6.根据视频的编码ID找到编码器
    avCodecParameters = av_fc->streams[video_stream_index]->codecpar;
    codec_decoder = avcodec_find_decoder(avCodecParameters->codec_id);
    if (codec_decoder == NULL) {
        LOGE("codec decoder not found");
        goto __END;
    }

    // 7.创建一个编解码器上下文
    avCodecContext = avcodec_alloc_context3(codec_decoder);
    if (avCodecContext == NULL) {
        LOGE("alloc avcodec context fail");
        goto __END;
    }

    ret = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    if (ret < 0) {
        LOGE("fill avcode context by media avcodec parameters failed");
        goto __END;
    }

    // 8. 打开解码器
    ret = avcodec_open2(avCodecContext, codec_decoder, NULL);
    if (ret != 0) {
        LOGE("open avcodec failed %d", ret);
        goto __END;
    }



    // 9 分配内存空间windowBuffer
    avPacket = av_packet_alloc(); // 用于保存从多媒体中读取的数据包
    frame = av_frame_alloc();      // 用于保存从编码器读出来的数据帧
    renderFrame = av_frame_alloc();// 用户保存转换成RGBA后的数据帧

    size = av_image_get_buffer_size(dstFormat, avCodecContext->width, avCodecContext->height,
                                    1);
    buffer = (uint8_t *) malloc(size * sizeof(uint8_t *));
    av_image_fill_arrays(renderFrame->data, renderFrame->linesize, buffer, dstFormat,
                         avCodecContext->width, avCodecContext->height, 1);

    // 10.创建swsContext, 用于格式转换
    // avCodecContext 默认是 AV_PIX_FMT_YUV420P; 这里的YUV420P是 I420格式的, YYYY UU VV
    LOGD("before swsContext %d %d, %d %d", avCodecContext->width, avCodecContext->height,
         avCodecContext->pix_fmt, AV_PIX_FMT_RGBA);
    swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, dstFormat,
            SWS_BILINEAR,
            NULL, NULL, NULL);

    if (swsContext == NULL) {
        LOGE("sws_getContext failed");
        goto __END;
    }

    // 11. 初始化ANativeWindow
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == NULL) {
        LOGE("create native window failed");
        goto __END;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow, avCodecContext->width, avCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);

    LOGD("before av_read_frame");
    flag = 1;
    // 12. 开始读取视频和渲染
    // av_read_frame: 0 if OK, < 0 on error or end of file
    while (av_read_frame(av_fc, avPacket) == 0 && flag) {
        // 如果读取到的数据包为视频
        if (avPacket->stream_index == video_stream_index) {
            // 发送给解码器
            int sendState = avcodec_send_packet(avCodecContext, avPacket);
            if (sendState == 0) {
                int receiveState = avcodec_receive_frame(avCodecContext, frame);
                // LOGD("receive frame %d", receiveState);
                if (receiveState == 0) {
                    sws_scale(swsContext,
                              (uint8_t const *const *) frame->data,
                              frame->linesize,
                              0,
                              avCodecContext->height, renderFrame->data, renderFrame->linesize);
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;
                    uint8_t *src = renderFrame->data[0];

                    int dst_stride = windowBuffer.stride * 4;
                    int src_stride = renderFrame->linesize[0];

                    for (int i = 0; i < avCodecContext->height; i++) {
                        memcpy(dst + i * dst_stride, src + i * src_stride, src_stride);
                    }

                    ANativeWindow_unlockAndPost(nativeWindow);
                } else {
                    LOGW("avcodec receive frame failed %d", receiveState);
                }
            } else {
                LOGW("avcodec send packet failed %d", sendState);
            }
        }
        av_packet_unref(avPacket);
    }

    __END:
    if (ret_avformat_open_input == 0) { avformat_close_input(&av_fc); }
    avformat_free_context(av_fc);
    if (avCodecContext != NULL) { avcodec_free_context(&avCodecContext); }
    if (avPacket != NULL) { av_packet_free(&avPacket); }
    if (frame != NULL) { av_frame_free(&frame); }
    if (renderFrame != NULL) { av_frame_free(&renderFrame); }
    if (nativeWindow != NULL) { ANativeWindow_release(nativeWindow); }
    if (avCodecContext != NULL) { avcodec_close(avCodecContext); }
    if (swsContext != NULL) { sws_freeContext(swsContext); }
    if (buffer != NULL) { free(buffer); }
    avformat_network_deinit();

    env->ReleaseStringUTFChars(videoPath_, video_path);
}

JNIEXPORT void JNICALL
nativeSaveStreamMedia(JNIEnv *env, jclass jclz, jstring rtp_url, jstring output_path) {

    const char *src_rtp_url = env->GetStringUTFChars(rtp_url, NULL);
    const char *output_file = env->GetStringUTFChars(output_path, NULL);

    // 1.注册编解码器
    avcodec_register_all();

    // 2.初始化网络
    avformat_network_init();

    // 3.创建AVFormatContext
    AVFormatContext *input_avfc = NULL, *output_avfc = NULL;
    AVOutputFormat *av_output_fmt = NULL;
    AVPacket avPacket;

    int ret = 0;
    int video_index = -1;
    int frame_index = 0;
    int64_t start_time;
    // 4. 打开输入文件或流，
    if (avformat_open_input(&input_avfc, src_rtp_url, NULL, NULL) < 0) {
        LOGD("could not open input ");
        goto __END;
    }
    if (avformat_find_stream_info(input_avfc, NULL) < 0) {
        LOGD("could not find stream info ");
        goto __END;
    }

    // 5.找到video_index;
    for (int i = 0; i < input_avfc->nb_streams; i++) {
        if (input_avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    }

    // av_dump_format(input_avfc,0,src_rtp_url,0);
    // 6.分配输出的上下文
    avformat_alloc_output_context2(&output_avfc, NULL, NULL, output_file);

    if (output_avfc == NULL) {
        LOGD("alloc output avformat context failed!");
        goto __END;
    }
    av_output_fmt = output_avfc->oformat;
    for (int i = 0; i < input_avfc->nb_streams; i++) {

        AVStream *in_stream = input_avfc->streams[i];
        AVStream *out_stream = avformat_new_stream(output_avfc, in_stream->codec->codec);
        if (out_stream == NULL) {
            LOGD("Failed allocating output stream!");
            goto __END;
        }

        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
            LOGD("Failed to copy cotext from input to output stream codec context");
            goto __END;
        }

        out_stream->codec->codec_tag = 0;
        if (output_avfc->oformat->flags & AVFMT_NOFILE) {
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    // av_dump_format(output_avfc,0,output_file,1);

    if (!(av_output_fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_avfc->pb, output_file, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGD("Could not open output URL: %s", output_file);
            goto __END;
        }
    }

    ret = avformat_write_header(output_avfc, NULL);
    if (ret < 0) {
        LOGD("Error occurred when opening output URL");
        goto __END;
    }

#if USE_H264BSF
    AVBitStremFilterContext *h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
    start_time = av_gettime();
    LOGD("before read frame");
    while (1) {

        AVStream *in_stream, *out_stream;
        ret = av_read_frame(input_avfc, &avPacket);
        if (ret < 0) break;
        if (avPacket.stream_index == video_index) {
            AVRational time_base = input_avfc->streams[video_index]->time_base;
            AVRational time_base_q = {1, AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(avPacket.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time) {
                av_usleep(pts_time - now_time);
            }
        }
        LOGD("after read frame:");
        in_stream = input_avfc->streams[avPacket.stream_index];
        out_stream = output_avfc->streams[avPacket.stream_index];

        // copy_packet
        // Convert PTS/DTS
        avPacket.pts = av_rescale_q_rnd(
                avPacket.pts,
                in_stream->time_base,
                out_stream->time_base,
                (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        avPacket.dts = av_rescale_q_rnd(
                avPacket.dts,
                in_stream->time_base,
                out_stream->time_base,
                (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        avPacket.duration = av_rescale_q(
                avPacket.duration,
                in_stream->time_base, out_stream->time_base);
        avPacket.pos = -1;

        if (avPacket.stream_index == video_index) {
            LOGD("Receiver %8d video frames from input URL", frame_index);
            frame_index++;
#if USE_H264BSF
            av_bitstream_filter_filter(
                    h264bsfc,
                    in_stream->codec,
                    NULL,
                    &avPacket.data,
                    &avPacket.size,
                    avPacket.data,
                    avPacket.size,0);
#endif
        }

        ret = av_interleaved_write_frame(output_avfc, &avPacket);
        if (ret < 0) {
            LOGD("Error muxing packet: %d %d %d", ret, avPacket.stream_index,
                 output_avfc->nb_streams);
            break;
        }
        av_free_packet(&avPacket);
    }
#if H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif

    av_write_trailer(output_avfc);
    __END:
    avformat_network_deinit();
    avformat_close_input(&input_avfc);

    // close output
    if (output_avfc && !(av_output_fmt->flags & AVFMT_NOFILE)) {
        avio_close(output_avfc->pb);
    }
    avformat_free_context(output_avfc);
    avformat_free_context(input_avfc);
    env->ReleaseStringUTFChars(rtp_url, src_rtp_url);
    env->ReleaseStringUTFChars(output_path, output_file);


}

void nativePlayStop(JNIEnv *env,jclass caller){
    flag = 0;
    LOGD("change flag value: %d",flag);
}

static JNINativeMethod method[] = {
        {"printFFmpegInfo",       "()V",                                                                          (void *) nativePrintFFmpegInfo},
        {"dumpMediaInfo",         "(Ljava/lang/String;)I",                                                        (void *) nativeDumpMediaInfo},
        {"nativePlayVideo",       "(Ljava/lang/String;Landroid/view/Surface;)V",                                  (void *) nativePlayVideo},
        {"nativePlayStream",      "(Ljava/lang/String;Landroid/view/Surface;Lorg/ffmpeg/utils/StreamCallback;)V", (void *) nativePlayStream},
        {"nativeSaveStreamMedia", "(Ljava/lang/String;Ljava/lang/String;)V",                                      (void *) nativeSaveStreamMedia},
        {"stopPlay",              "()V",                                                                          (void *) nativePlayStop}
};

int register_jni(JNIEnv *env) {

    jclass cls = env->FindClass(CLASS_NAME);

    if (cls == NULL) {
        LOGE("FindClass fail %s", CLASS_NAME);
        return JNI_ERR;
    }

    int ret = -1;
    if ((ret = env->RegisterNatives(cls, method, ARRAY_LENGTH(method))) < 0) {
        LOGE("RegisterNatives fail %d", ret);
        return JNI_ERR;
    }
    return JNI_OK;
}
