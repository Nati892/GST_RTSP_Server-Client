#include <iostream>
#include <thread>
#include <atomic>
#include "Main.h"
#include <gst/gst.h>

bool running = true;
bool movie_paused = false;
GstElement* pipeline;
gint64 position;


//input processing loop
void capture_input()
{
	while (running) {
		char c = std::cin.get();
		if (pipeline == nullptr)
			continue;

		switch (c)
		{
		case ' ':
			if (movie_paused)
			{
				std::cout << "playing" << std::endl;
				gst_element_set_state(pipeline, GST_STATE_PLAYING);
				movie_paused = false;
			}
			else
			{
				std::cout << "movie paused" << std::endl;
				gst_element_set_state(pipeline, GST_STATE_PAUSED);
				movie_paused = true;
			}
			break;

		case 'b':
			if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position)) {
				gint64 seek_time = position - 10 * GST_SECOND;
				if (seek_time < 0) seek_time = 0; // Ensure we don't seek before 0:00 time
				std::cout << "Seeking backward to " << seek_time / GST_SECOND << " seconds." << std::endl;
				gst_element_seek_simple(pipeline, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), seek_time);
			}
			break;

		case 'f':
			if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position)) {
				gint64 seek_time = position + 10 * GST_SECOND;
				std::cout << "Seeking forward to " << seek_time / GST_SECOND << " seconds." << std::endl;
				gst_element_seek_simple(pipeline, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), seek_time);
			}

			break;

		default:
			break;
		}

	}
}


void play_rtsp_client(const std::string& rtsp_url) {
	GstBus* bus;
	GstMessage* msg;

	/* Initialize GStreamer */
	gst_init(nullptr, nullptr);

	/* Create the pipeline */
	std::string pipeline_description = "playbin uri=" + rtsp_url;
	pipeline = gst_parse_launch(pipeline_description.c_str(), nullptr);

	if (!pipeline) {
		std::cerr << "Failed to create pipeline for RTSP stream!" << std::endl;
		return;
	}

	/* Start playing the RTSP stream */
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	/* Separate thread to handle user input */
	std::thread input_thread(capture_input);

	/* Wait until error or EOS */
	bus = gst_element_get_bus(pipeline);
	msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

	/* Handle errors or end of stream */
	if (msg != nullptr) {
		if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
			GError* err;
			gchar* debug_info;
			gst_message_parse_error(msg, &err, &debug_info);
			std::cerr << "Error received: " << err->message << std::endl;
			g_clear_error(&err);
			g_free(debug_info);
		}
		else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
			std::cout << "End of stream reached." << std::endl;
		}
		gst_message_unref(msg);
	}

	/* Stop the input thread */
	running = false;
	input_thread.join();

	/* Clean up */
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
}

int main(int argc, char** argv)
{
	std::cout << "hello" << std::endl;
	if (argc < 2)
	{
		std::cerr << "missing source steram url parameter" << std::endl;
		return 1;
	}
	play_rtsp_client(argv[1]);

	return 0;
}
