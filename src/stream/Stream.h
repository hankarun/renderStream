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
    
    
    /**
     * Initialize the stream with the given output destination, dimensions and frame rate
     * 
     * @param outputFile Path to output file or RTSP URL (e.g., "rtsp://localhost:8554/live")
     * @param width Width of the video stream
     * @param height Height of the video stream
     * @param fps Frames per second
     * @return true if initialization succeeded, false otherwise
     */
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
