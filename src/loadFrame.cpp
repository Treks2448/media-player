extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <inttypes.h>
	#include <libswscale/swscale.h>
}



bool loadFrame(const char* filename, int* width, int* height, unsigned char** data) {
    
    // struct containing data about file format
    AVFormatContext* format_context = avformat_alloc_context();
    if (!format_context) {
		printf("Couldn't create AVFormatContext\n");
		return false;
    }
    
    // load data about file format into the context
    if (avformat_open_input(&format_context, filename, NULL, NULL) != 0) {
		printf("Couldn't open video file\n");
		return false;
    }
	
	// read additional stream information
	if (avformat_find_stream_info(format_context, NULL) < 0) {
		printf("Could not retreive stream information from the file\n");
		return false;
	}
 
    // get the video stream (index)
    int stream_idx = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream* stream = format_context->streams[stream_idx]; 
    if (stream_idx < 0) {
		printf("Could not find stream of type AVMEDIA_TYPE_VIDEO\n");
		return false;
    }

    // get the respective decoder 
    AVCodec* video_decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!video_decoder) {
		printf("NULL video decoder");
    }

    // struct containing details about decoder
    AVCodecContext* decoder_context = avcodec_alloc_context3(video_decoder);
    if (!decoder_context) {
		printf("Could not allocate codec context\n");
		return false;
    }
	
	// fill decoder context with parameters relative to stream	
	if (avcodec_parameters_to_context(decoder_context, stream->codecpar) < 0) {
		printf("Failed to fill decoder context with stream parameters\n");
		return false;
	}	
 
    // Initialize the decoder 
    if (avcodec_open2(decoder_context, video_decoder, NULL) != 0) {
		printf("Could not initialize the decoder\n");
		return false;
    }
    
    // Allocate packet and frame
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
		printf("Couldn't allocate packet\n");
		return false;
    }
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
		printf("Couldn't allocate frame\n");
		return false;
    }

    // read a single video frame from the file
    while (av_read_frame(format_context, packet) >= 0) {
		// skip non-video packets	
		if (packet->stream_index != stream_idx) {
		   continue; 
		}

		// send packet to the decoder	
		int result = avcodec_send_packet(decoder_context, packet);
		if (result < 0) {
		    printf("Couldn't send packet for decoding\n");
		    return false;
		}

		// get a frame from the decoder
		result = avcodec_receive_frame(decoder_context, frame);
		if (result == AVERROR_EOF || result == AVERROR(EAGAIN)) {
		    printf("EOF or EAGAIN\n"); 
		    continue; // TODO: temporary, change this
		}
		else if (result < 0) {
		    printf("Error during decoding\n");
		    return false;
		}	

		// unreference the packet
		av_packet_unref(packet); 
		
		break;
    }
	
	// Initialize scaler for converting pixel format	
	SwsContext* scaler_ctx = sws_getContext(
		frame->width, 
		frame->height, 
		decoder_context->pix_fmt, // input pixel format
		frame->width,
		frame->height,
		AV_PIX_FMT_RGB0,   // output pixel format
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);	
	if (!scaler_ctx) {
		printf("Couldn't initialize scaler context\n");
		return false;
	}
	
	// Set up new pixel format data containers
	uint8_t* RGBA_pixels = new uint8_t[frame->width * frame->height * 4];
	uint8_t* pix_buff[4] = {RGBA_pixels, NULL, NULL, NULL};
	int RGBA_linesize[4] = {frame->width * 4, 0, 0, 0};
		
	// Process pixel format conversion	
	sws_scale(scaler_ctx, frame->data, frame->linesize, 0, frame->height, pix_buff, RGBA_linesize);
	sws_freeContext(scaler_ctx); 

    *width = frame->width;
    *height = frame->height;
    *data = RGBA_pixels;
	    
    // close input file and clean up memory
    av_frame_free(&frame);
    av_packet_free(&packet);
    avformat_close_input(&format_context);
    avformat_free_context(format_context);
    avcodec_free_context(&decoder_context); 

    return true;
}

//TODO: break up the loadFrame function into additional functions
