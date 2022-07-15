#include "AVPlayer.h"

/* AVPLayerState functions*/

/* Allocates a new AVPlayerState and returns a pointer to it */
AVPlayerState* allocAVPlayerState( enum AVMediaType media_type, AVFormatContext* format_context
) {
	// Create an AVPlayerState	
	struct AVPlayerState* av_player_state = (AVPlayerState*)malloc(sizeof *av_player_state);
	av_player_state->codec = NULL;
	av_player_state->codec_context = NULL;
	av_player_state->frame = NULL;
	av_player_state->packet = NULL;
	av_player_state->scaler_ctx = NULL;	
	av_player_state->data = NULL;

	// Set media type
	av_player_state->media_type = media_type;
	
	// Get the video stream (index)
    av_player_state->media_stream_idx = av_find_best_stream(
		format_context, av_player_state->media_type, -1, -1, NULL, 0
	);
    if (av_player_state->media_stream_idx < 0) {
		printf("Could not find stream of type AVMEDIA_TYPE_VIDEO\n");
		return NULL;
    }	
    AVStream* stream = format_context->streams[av_player_state->media_stream_idx]; 

    // Get the respective codec 
    av_player_state->codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!av_player_state->codec) {
		printf("NULL video decoder\n");
		return NULL;
    }
	
	// Allocate memory for codec context
    av_player_state->codec_context = avcodec_alloc_context3(av_player_state->codec);
    if (!av_player_state->codec_context) {
		printf("Could not allocate codec context\n");
		return NULL;
    }

	// Allocate packet 
    av_player_state->packet = av_packet_alloc();
    if (!av_player_state->packet) {
		printf("Couldn't allocate packet\n");
		return NULL;
    }

	// Allocate frame
    av_player_state->frame = av_frame_alloc();
    if (!av_player_state->frame) {
		printf("Couldn't allocate frame\n");
		return NULL;
    }	

	return av_player_state;
}

// Initializes the AVPlayerState
int initAVPlayerState(AVPlayerState* av_player_state, AVFormatContext* format_context) {
	// fill decoder context with parameters relative to stream	
	if (avcodec_parameters_to_context(
		av_player_state->codec_context,
		format_context->streams[av_player_state->media_stream_idx]->codecpar) < 0
	) {
		printf("Failed to fill decoder context with stream parameters\n");
		return -1;
	}	
 
    // Initialize the decoder 
    if (avcodec_open2(av_player_state->codec_context, av_player_state->codec, NULL) != 0) {
		printf("Could not initialize the decoder\n");
		return -1;
	}	

	if (av_player_state->media_type == AVMEDIA_TYPE_VIDEO) {
		// Set video dimensions	
		av_player_state->width = av_player_state->codec_context->width;
		av_player_state->height = av_player_state->codec_context->height;
		// Create a scaler (to conver to RGBA format)	
		av_player_state->scaler_ctx = sws_getContext(
			av_player_state->width, 
			av_player_state->height, 
			av_player_state->codec_context->pix_fmt, // input pixel format
			av_player_state->width,
			av_player_state->height,
			AV_PIX_FMT_RGB0,   // output pixel format
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL
		);	
		if (!av_player_state->scaler_ctx) {
			printf("Couldn't initialize scaler context\n");
			return -1;
		}
	}	

	return 0;
}

int nextFrame(AVPlayerState* av_player_state, AVFormatContext* format_context) {
	// read a single video frame from the file
    while (av_read_frame(format_context, av_player_state->packet) >= 0) {
		// skip non-video packets	
		if (av_player_state->packet->stream_index != av_player_state->media_stream_idx) {
		   continue; 
			av_packet_unref(av_player_state->packet); 
		}

		// send packet to the decoder	
		int result = avcodec_send_packet(av_player_state->codec_context, av_player_state->packet);
		if (result < 0) {
		    printf("Couldn't send packet for decoding\n");
		    return -1;
		}

		// get a frame from the decoder
		result = avcodec_receive_frame(av_player_state->codec_context, av_player_state->frame);
		if (result == AVERROR_EOF || result == AVERROR(EAGAIN)) {
		    continue; 
			av_packet_unref(av_player_state->packet); 
		}
		else if (result < 0) {
		    printf("Error during decoding\n");
		    return -1;
		}	

		// unreference the packet
		av_packet_unref(av_player_state->packet); 
		
		break;
    }
	
	if (av_player_state->media_type == AVMEDIA_TYPE_VIDEO) {
		// Set up pixel data containers if they haven't been set up yet
		if (!av_player_state->data) {
			av_player_state->data = (uint8_t*)malloc(
				sizeof(uint8_t) * av_player_state->frame->width * av_playeav_player_state->frame->height * 4
			);
			av_player_state->pix_buff[0] = av_player_state->data;
			av_player_state->RGBA_linesize[0] = av_player_state->frame->width * 4;
		}	
			
		// Process pixel format conversion	
		sws_scale(
			av_player_state->scaler_ctx, 
			av_player_state->frame->data, 
			av_player_state->frame->linesize, 
			0, 
			av_player_state->frame->height, 
			av_player_state->pix_buff, 
			av_player_state->RGBA_linesize
		);	
	}	
	
	return 0;
}



// Frees and cleans up given AVPlayerState 
void freeAVPlayerState(AVPlayerState* av_player_state) {	
	free(av_player_state->data);
	sws_freeContext(av_player_state->scaler_ctx); 
	av_frame_free(&av_player_state->frame);
    av_packet_free(&av_player_state->packet);
    avcodec_free_context(&av_player_state->codec_context);	
	free(av_player_state);	
}

/* AVPlayer functions */

/* Allocates a new AVPlayer and returns a pointer to it*/
AVPlayer* allocAVPlayer() {
	// Create an AVPlayer	
	struct AVPlayer* av_player = (AVPlayer*)malloc(sizeof *av_player);
	av_player->audio_state = NULL;
	av_player->video_state = NULL;	

	// Allocate format context
	av_player->format_context = avformat_alloc_context();
    if (!av_player->format_context) {
		printf("Couldn't create AVFormatContext\n");
		return NULL;
    }	
	// TODO: error checking on allocation of audio and video state
		
	return av_player;
}

/* Opens a new file for use by the given AVPlayer */ 
int AVPlayerOpen(const char* filename, AVPlayer* av_player) {
    // load data about file format into the context
    if (avformat_open_input(&(av_player->format_context), filename, NULL, NULL) != 0) {
		printf("Couldn't open video file\n");
		return -1;
    }
	
	// read additional stream information
	if (avformat_find_stream_info(av_player->format_context, NULL) < 0) {
		printf("Could not retreive stream information from the file\n");
		return -1;
	}

	return 0;
}

/* Closes the file that was being operated on by the given AVPlayer */
void AVPlayerClose(AVPlayer* av_player) {
	avformat_close_input(&(av_player->format_context));
}

/* Deallocates and cleans up resources used by AVPlayer. AVPlayerClose must be called on the given
 * AVPlayer before it is freed.*/
void freeAVPlayer(AVPlayer* av_player) {	
	if (av_player->video_state) { freeAVPlayerState(av_player->video_state); }
	if (av_player->audio_state) { freeAVPlayerState(av_player->audio_state); }
	avformat_free_context(av_player->format_context);	
	free(av_player);
}


