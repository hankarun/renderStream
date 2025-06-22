#include "Stream.h"
#include <iostream>

Stream::Stream() 
    : formatContext(nullptr), codecContext(nullptr), stream(nullptr), 
      frame(nullptr), packet(nullptr), swsContext(nullptr),
      frameCount(0), width(0), height(0), isInitialized(false) {
    // Initialize FFmpeg's network capabilities at constructor
    avformat_network_init();
}

Stream::~Stream() {
    finalize();
}

bool Stream::initialize(const std::string& outputFile, int width, int height, int fps) {
    this->width = width;
    this->height = height;
    this->frameCount = 0;
    
    // Make sure network is initialized for streaming
    static int networkInitialized = 0;
    if (!networkInitialized) {
        int ret = avformat_network_init();
        if (ret < 0) {
            std::cerr << "Failed to initialize FFmpeg network components" << std::endl;
            return false;
        }
        networkInitialized = 1;
    }
    
    // Determine if we're streaming via RTSP or writing to a file
    bool isRtspStream = outputFile.compare(0, 7, "rtsp://") == 0;
    
    // Initialize format context for output
    int ret = 0;
    if (isRtspStream) {
        // For RTSP, we need to explicitly use the RTSP output format
        ret = avformat_alloc_output_context2(&formatContext, NULL, "rtsp", outputFile.c_str());
    } else {
        // For files, let FFmpeg auto-detect the format based on the file extension
        ret = avformat_alloc_output_context2(&formatContext, NULL, NULL, outputFile.c_str());
    }
    
    if (!formatContext) {
        std::cerr << "Could not create output context (error code: " << ret << ")" << std::endl;
        return false;
    }
    
    // Find the encoder - use H264 for compatibility
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "Could not find encoder" << std::endl;
        return false;
    }
    
    // Create a new video stream
    stream = avformat_new_stream(formatContext, codec);
    if (!stream) {
        std::cerr << "Could not create video stream" << std::endl;
        return false;
    }
    
    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return false;
    }
    
    // Configure codec parameters
    codecContext->codec_id = codec->id;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = AVRational{1, fps};
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->bit_rate = 2000000; // 2 Mbps
    
    // RTSP streaming specific settings
    if (isRtspStream) {
        // Lower latency settings for streaming
        codecContext->gop_size = 12; // Keyframe every 12 frames
        codecContext->max_b_frames = 0; // No B-frames for lower latency
        codecContext->rc_buffer_size = codecContext->bit_rate; // Buffer size
        codecContext->rc_max_rate = codecContext->bit_rate;
        codecContext->rc_min_rate = codecContext->bit_rate;
        
        // Set RTSP transport protocol options
        av_dict_set(&formatContext->metadata, "rtsp_transport", "tcp", 0); // Use TCP for more reliable streaming
    }
    
    // Some formats require specific parameters
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    // Open codec
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return false;
    }
    
    // Copy codec parameters to stream
    if (avcodec_parameters_from_context(stream->codecpar, codecContext) < 0) {
        std::cerr << "Could not copy codec parameters to stream" << std::endl;
        return false;
    }
    
    // Allocate frame and packet
    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate video frame" << std::endl;
        return false;
    }
    
    frame->format = codecContext->pix_fmt;
    frame->width = width;
    frame->height = height;
    
    if (av_frame_get_buffer(frame, 0) < 0) {
        std::cerr << "Could not allocate frame data" << std::endl;
        return false;
    }
    
    packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Could not allocate packet" << std::endl;
        return false;
    }
    
    // Create SwsContext for pixel format conversion
    swsContext = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                               width, height, AV_PIX_FMT_YUV420P,
                               SWS_BICUBIC, NULL, NULL, NULL);
    
    if (!swsContext) {
        std::cerr << "Could not initialize SwsContext" << std::endl;
        return false;
    }
    
    // Open output - different for file vs. network streaming
    AVDictionary* options = NULL;
    if (isRtspStream) {
        // For RTSP streaming
        av_dict_set(&options, "rtsp_transport", "tcp", 0); // Use TCP (more reliable than UDP)
        av_dict_set(&options, "muxdelay", "0", 0); // Minimize delay
        
        // Additional RTSP settings that might help
        av_dict_set(&options, "fflags", "nobuffer", 0); // Reduce latency
        av_dict_set(&options, "max_delay", "500000", 0); // 0.5 second max delay
        av_dict_set(&options, "tune", "zerolatency", 0); // Tune for low latency
    }
    
    // Open the output
    int avio_ret = 0;
    if (isRtspStream) {
        // Make sure we're set to RTSP output format
        formatContext->oformat = av_guess_format("rtsp", NULL, NULL);
        if (!formatContext->oformat) {
            std::cerr << "Could not find RTSP output format" << std::endl;
            return false;
        }
    }
    
    avio_ret = avio_open2(&formatContext->pb, outputFile.c_str(), AVIO_FLAG_WRITE, NULL, &options);
    if (avio_ret < 0) {
        char errbuf[256];
        av_strerror(avio_ret, errbuf, sizeof(errbuf));
        std::cerr << "Could not open output: " << outputFile << " (" << errbuf << ")" << std::endl;
        return false;
    }
    
    // Write header
    int header_ret = avformat_write_header(formatContext, &options);
    if (header_ret < 0) {
        char errbuf[256];
        av_strerror(header_ret, errbuf, sizeof(errbuf));
        std::cerr << "Could not write header: " << errbuf << std::endl;
        return false;
    }
    
    if (options) {
        av_dict_free(&options);
    }
    
    isInitialized = true;
    return true;
}

bool Stream::addFrame(const unsigned char* data) {
    if (!isInitialized) {
        std::cerr << "Stream not initialized" << std::endl;
        return false;
    }
    
    // Make sure frame is writable
    if (av_frame_make_writable(frame) < 0) {
        std::cerr << "Could not make frame writable" << std::endl;
        return false;
    }
    
    // Set up source buffer (RGB)
    const uint8_t* srcSlice[1] = { data };
    int srcStride[1] = { 3 * width }; // RGB stride
    
    // Convert RGB to YUV
    sws_scale(swsContext, srcSlice, srcStride, 0, height,
              frame->data, frame->linesize);
    
    // Set frame timestamp
    frame->pts = frameCount++;
    
    // Encode the frame
    int ret = avcodec_send_frame(codecContext, frame);
    if (ret < 0) {
        std::cerr << "Error sending frame to codec" << std::endl;
        return false;
    }
    
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            std::cerr << "Error receiving packet from codec" << std::endl;
            return false;
        }
        
        // Rescale packet timestamps
        av_packet_rescale_ts(packet, codecContext->time_base, stream->time_base);
        packet->stream_index = stream->index;
        
        // Write packet to output file
        ret = av_interleaved_write_frame(formatContext, packet);
        if (ret < 0) {
            std::cerr << "Error writing packet" << std::endl;
            return false;
        }
    }
    
    return true;
}

void Stream::finalize() {
    if (isInitialized) {
        // Flush encoder
        int ret = avcodec_send_frame(codecContext, NULL);
        if (ret >= 0) {
            while (avcodec_receive_packet(codecContext, packet) >= 0) {
                av_packet_rescale_ts(packet, codecContext->time_base, stream->time_base);
                packet->stream_index = stream->index;
                av_interleaved_write_frame(formatContext, packet);
            }
        }
        
        // Write trailer
        av_write_trailer(formatContext);
        
        // Close output file or stream
        if (formatContext && !(formatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&formatContext->pb);
        }
    }
    
    // Free resources
    if (swsContext) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
    
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    
    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }
    
    if (formatContext) {
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
    
    isInitialized = false;
}
