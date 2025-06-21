#ifndef STREAM_H
#define STREAM_H

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

class Stream {
public:
    Stream();
    ~Stream();
    
    bool initialize(const std::string& outputFile, int width, int height, int fps);
    bool addFrame(const unsigned char* data);
    void finalize();
    
private:
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVStream* stream;
    AVFrame* frame;
    AVPacket* packet;
    SwsContext* swsContext;
    
    int frameCount;
    int width;
    int height;
    bool isInitialized;
};

#endif // STREAM_H
