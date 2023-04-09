//
// Created by Usama Liaqat on 05/04/2023.
//

#ifndef GSTREAMER_BUFFER_H
#define GSTREAMER_BUFFER_H

#include <gst/gst.h>
#include "string"
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <cstring>
#include <utility>

using namespace std;

typedef struct CustomData {
    GMainLoop *main_loop;
    GstElement *pipeline;
    GstElement *source;
    GstElement *appsink;
    GstElement *depay;
    GstElement *filter;
} CustomData;

class BufferReader {
public:
    string sourceUrl;
    BufferReader(const char string[27]);

    static GstFlowReturn on_new_sample(GstElement *sink, CustomData *data);
    static void on_rtsp_pad_created(GstElement *element, GstPad *pad, gpointer data);
    int startAppSink();

};


#endif //GSTREAMER_BUFFER_H
