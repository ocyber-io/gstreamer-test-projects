//
// Created by Usama Liaqat on 05/04/2023.
//

#include "buffer.h"

int BufferReader::startAppSink() {
    CustomData data;

    gst_init(nullptr, nullptr);
    string app_sink_name = "rtsp-sink";
    string stream_name = "fox-and-bird";
    string stream_url = sourceUrl;
    GstStateChangeReturn ret;
    memset(&data, 0, sizeof(data));

    data.source = gst_element_factory_make("rtspsrc", "source");
    data.appsink = gst_element_factory_make("appsink", app_sink_name.c_str());
    data.depay = gst_element_factory_make("rtph264depay", "depay");
    data.filter = gst_element_factory_make("capsfilter", "encoder_filter");
    GstCaps *h264_caps = gst_caps_new_simple("video/x-h264",
                                             "stream-format", G_TYPE_STRING, "avc",
                                             "alignment", G_TYPE_STRING, "au",
                                             NULL);
    g_object_set(G_OBJECT (data.filter), "caps", h264_caps, NULL);
    gst_caps_unref(h264_caps);
    data.pipeline = gst_pipeline_new("rtsp-pipeline");

    if (!data.pipeline || !data.source || !data.depay || !data.appsink) {
        g_printerr("Not all elements could be created:\n");
        if (!data.pipeline) g_printerr("\tCore pipeline\n");
        if (!data.source) g_printerr("\trtspsrc (gst-plugins-good)\n");
        if (!data.depay) g_printerr("\trtph264depay (gst-plugins-good)\n");
        if (!data.appsink) g_printerr("\tappsink (gst-plugins-base)\n");
        return 1;
    }

    g_object_set(G_OBJECT (data.source),
                 "location", stream_url.c_str(),
                 "short-header", true, // Necessary for target camera
                 NULL);
    g_object_set(G_OBJECT (data.appsink), "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect(data.appsink, "new-sample", G_CALLBACK(BufferReader::on_new_sample), &data);
    g_signal_connect(data.source, "pad-added", G_CALLBACK(BufferReader::on_rtsp_pad_created), data.depay);

    gst_bin_add_many(GST_BIN (data.pipeline), data.source,
                     data.depay, data.filter, data.appsink,
                     NULL);

    if (!gst_element_link_many(data.depay, data.filter,
                               data.appsink,
                               NULL)) {

        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.pipeline);
        return 1;
    }

    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        return 1;
    }

    data.main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(data.main_loop);
}


void BufferReader::on_rtsp_pad_created(GstElement *element, GstPad *pad, gpointer data) {
    gchar *pad_name = gst_pad_get_name(pad);
    GstElement *other = reinterpret_cast<GstElement *>(data);
    g_print("New RTSP source found: %s\n", pad_name);
    if (gst_element_link(element, other)) {
        g_print("Source linked.\n");
    } else {
        g_print("Source link FAILED\n");
    }
    g_free(pad_name);
}

GstFlowReturn BufferReader::on_new_sample(GstElement *sink, CustomData *data) {
    GstSample *sample = NULL;
    GstBuffer *buffer = NULL;
    gpointer copy = NULL;
    gsize copy_size = 0;
    g_signal_emit_by_name (sink, "pull-sample", &sample);
    if (sample) {
        g_print("Source on_new_sample.\n");
        buffer = gst_sample_get_buffer(sample);
        if (buffer) {
            gst_buffer_extract_dup(buffer, 0, gst_buffer_get_size(buffer), &copy, &copy_size);
        }
        gst_sample_unref (sample);
    }

    return GST_FLOW_OK;

}

BufferReader::BufferReader(const char *url) {
    sourceUrl = url;
}
