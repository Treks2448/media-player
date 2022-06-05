extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <inttypes.h>
}

/* TODO this isn't accuraten, need to update
In ffmpeg, the process of getting a data stream from a file goes like this:
1.  Create a "context" that holds information about the file/file type: AVFormatContext.
    Examples of information stored here are the various audio/video streams and metadata.
2.  Find a stream of a certain media type, for example a video stream. The function 
    av_find_best_stream() handles this when given an AVFormatContext and the desired media type. 
     - The function returns a stream index which is used to access the stream within an array of
       streams in the file.
     - The function also optionally returns an appropriate decoder for the media type.
3.  Create a decoder context for the stream. The context holds details about the decoder:
    AVCodecContext.
4.  A video file contains multiplexed data as a series of packets, for example:
    file data:  [[video-data][audio-data][other-data]...[video-datal]...[other-data]]. If we wish
    to, for example, extract video data, we must specifically look for video packets. We may
    extract data such as video frames from a video packet by using the created decoder.
*/

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
   
    // build the pixel array respective of the frame
	// TODO: linesize may be negative in which case this part of the program crashes. 
    unsigned char* frame_pixels = new unsigned char[frame->width * frame->height * 3];
    for (int x = 0; x < frame->width; x++) {
		for (int y = 0; y < frame->height; y++) {
			frame_pixels[y * frame->width * 3 + x * 3 + 0] = frame->data[0][y * frame->linesize[0] + x];
	    	frame_pixels[y * frame->width * 3 + x * 3 + 1] = frame->data[0][y * frame->linesize[0] + x];
	    	frame_pixels[y * frame->width * 3 + x * 3 + 2] = frame->data[0][y * frame->linesize[0] + x];
		}
    }

    printf("width %d, height %d\n", frame->width, frame->height); // TODO: for testing, remove later
    *width = frame->width;
    *height = frame->height;
    *data = frame_pixels;
	    
    // close input file and clean up memory
    av_frame_free(&frame);
    av_packet_free(&packet);
    avformat_close_input(&format_context);
    avformat_free_context(format_context);
    avcodec_free_context(&decoder_context); 

    return true;
}
