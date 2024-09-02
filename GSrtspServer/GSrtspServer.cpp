#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

static gboolean AddMountPoints(GstRTSPServer* server) {
	GstRTSPMountPoints* mounts;
	GstRTSPMediaFactory* factory0;
	GstRTSPMediaFactory* factory1;

	// Create a new mount point for the server
	mounts = gst_rtsp_server_get_mount_points(server);

	// Create a factory for a single video file
	factory0 = gst_rtsp_media_factory_new();
	gst_rtsp_media_factory_set_launch(factory0,
		"( filesrc location=mov1.mp4 ! qtdemux ! h264parse ! rtph264pay name=pay0 pt=96 )");

	factory1 = gst_rtsp_media_factory_new();
	gst_rtsp_media_factory_set_launch(factory1,
		"( filesrc location=mov0.mp4 ! qtdemux ! h264parse ! rtph264pay name=pay0 pt=96 )");
	// Attach the factory to the mount point
	gst_rtsp_mount_points_add_factory(mounts, "/test0", factory0);
	gst_rtsp_mount_points_add_factory(mounts, "/test1", factory1);

	// Unreference the mounts object
	g_object_unref(mounts);

	return TRUE;
}

int main(int argc, char* argv[]) {
	GstRTSPServer* server;
	GMainLoop* loop;

	gst_init(&argc, &argv);

	// Create a new RTSP server instance
	server = gst_rtsp_server_new();

	AddMountPoints(server);

	// Attach the server to the default main context
	gst_rtsp_server_attach(server, NULL);

	// Create a new GMainLoop object
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	// Cleanup
	g_object_unref(server);
	g_main_loop_unref(loop);

	return 0;
}
