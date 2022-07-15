extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <inttypes.h>
	#include <libswscale/swscale.h>
}

// AVPlayerState /////////////////////////////////////////////

struct AVPlayerState {
	AVMediaType media_type;
	int media_stream_idx;	
	AVCodec* codec = NULL;
	AVCodecContext* codec_context = NULL;
	AVFrame* frame = NULL;
	AVPacket* packet = NULL;
	
	/* Video only */
	int width;
	int height;	
	SwsContext* scaler_ctx = NULL;	
	uint8_t* data = NULL;
	int RGBA_linesize[4] = {0, 0, 0, 0};
	uint8_t* pix_buff[4] = {data, NULL, NULL, NULL};
};

AVPlayerState* allocAVPlayerState(enum AVMediaType media_type, AVFormatContext* format_context);
int initAVPlayerState(AVPlayerState* av_player_state, AVFormatContext* format_context);
int nextFrame(AVPlayerState* av_player_state, AVFormatContext* format_context);
void freeAVPlayerState(AVPlayerState* av_player_state);

// AVPlayer ////////////////////////////////////////////////// 

struct AVPlayer {
	AVFormatContext* format_context = NULL;
	AVPlayerState* audio_state = NULL;
	AVPlayerState* video_state = NULL;
};

AVPlayer* allocAVPlayer();
int AVPlayerOpen(const char* filename, AVPlayer* av_player);
void AVPlayerClose(AVPlayer* av_player);
void freeAVPlayer(AVPlayer* av_player);
