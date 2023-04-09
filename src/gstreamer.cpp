//
// Created by Usama Liaqat on 29/03/2023.
//

#include "gstreamer.h"
#include <iostream>
#include <utility>

using namespace std;

typedef struct SourceData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *rtpjitterbuffer;
    GstElement *depay;
    GstElement *filter;
    GstElement *parse;
    GstElement *pay;
    GstElement *sink;
    GstBus *bus;
    GstStateChangeReturn ret;
    guint bus_watch_id;
} SourceData;


GStreamer::GStreamer(std::string url) {
    this->sourceUrl = std::move(url);
}

gboolean GStreamer::sourceBus(GstBus *bus, GstMessage *msg, gpointer data) {
    auto *gMainLoop = (GMainLoop *) data;
    const gchar *message_type_name = gst_message_type_get_name(GST_MESSAGE_TYPE(msg));
//    g_print("bus_callback => Received message type: %s\n", message_type_name);

    switch (GST_MESSAGE_TYPE (msg)) {

        case GST_MESSAGE_EOS:
            fprintf(stderr, "End of stream\n");
            g_main_loop_quit(gMainLoop);
            break;

        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;

            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);

            g_printerr("Error: %s\n", error->message);
            g_error_free(error);

            g_main_loop_quit(gMainLoop);

            break;
        }
        default:
            break;
    }

    return TRUE;
}

void GStreamer::onPadAdded(GstElement *element, GstPad *pad, gpointer data) {
    auto *other = reinterpret_cast<GstElement *>(data);
    gchar *pad_name = gst_pad_get_name(pad);
    GstCaps *caps = gst_pad_get_current_caps(pad);
    GstCaps *new_pad_caps = gst_pad_get_current_caps(pad);

    GstStructure *rtp_caps_struct = gst_caps_get_structure(new_pad_caps, 0);
    const gchar *media_type = gst_structure_get_string(rtp_caps_struct, "media");

    g_print("New RTSP Media Type: %s\n", pad_name);
    if (g_str_has_prefix(gst_caps_to_string(caps), "application/x-rtp")) {
        if (g_strcmp0(media_type, "video") == 0) {
            gint payload_type;
            gst_structure_get_int(rtp_caps_struct, "payload", &payload_type);
            g_print("Video pad is %d.\n", payload_type);

            if (gst_element_link(element, other)) {
                g_print("Source linked.\n");
            } else {
                g_print("Source link FAILED\n");
                return;
            }
        }
    }
    g_free(pad_name);
}

int GStreamer::startSource() {
    SourceData data;


    gst_init(nullptr, nullptr);
    g_setenv("GST_DEBUG", "3", TRUE);

    data.source = gst_element_factory_make("rtspsrc", "rtsp-source");
    data.rtpjitterbuffer = gst_element_factory_make("rtpjitterbuffer", "rtpjitterbuffer");
    data.depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    data.parse = gst_element_factory_make("h264parse", "h264parse");
    data.pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    data.filter = gst_element_factory_make("capsfilter", "encoder_filter");
    data.sink = gst_element_factory_make("udpsink", "udp-sink");

    GstCaps *h264_caps = gst_caps_new_simple("video/x-h264",
                                             "stream-format", G_TYPE_STRING, "avc",
                                             "alignment", G_TYPE_STRING, "au",
                                             NULL);
    g_object_set(G_OBJECT (data.filter), "caps", h264_caps, NULL);
    gst_caps_unref(h264_caps);

    g_object_set(G_OBJECT(data.source), "location", this->sourceUrl.c_str(), "short-header", true, NULL);
    g_object_set(G_OBJECT(data.sink), "port", 5000, NULL);

    data.pipeline = gst_pipeline_new("rtsp-pipeline");

    g_signal_connect(data.source, "pad-added", G_CALLBACK(GStreamer::onPadAdded), data.depay);


    if (!data.pipeline || !data.source || !data.depay || !data.parse || !data.pay ||
        !data.sink) {
        g_printerr("Failed to create pipeline elements.\n");
        return -1;
    }


    gst_bin_add_many(GST_BIN (data.pipeline), data.source, data.depay, data.filter, data.parse, data.pay, data.sink,
                     NULL);

    if (
            !gst_element_link_many( data.depay, data.filter, data.parse, data.pay, data.sink, NULL)
            ) {
        g_printerr("Failed to link elements. \n");
        gst_object_unref(data.pipeline);
        return -1;
    }


    data.ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (data.ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to start pipeline");
        gst_object_unref(data.pipeline);
        return -1;
    }
    loop = g_main_loop_new(NULL, FALSE);
    data.bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));
    data.bus_watch_id = gst_bus_add_watch(data.bus, GStreamer::sourceBus, loop);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);

    return 0;
}

int GStreamer::startPublish() {
    return 0;
}
