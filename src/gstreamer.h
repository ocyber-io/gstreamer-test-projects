//
// Created by Usama Liaqat on 29/03/2023.
//

#ifndef GSTREAMER_GSTREAMER_H
#define GSTREAMER_GSTREAMER_H

#include <gst/gst.h>
#include "string"
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

class GStreamer {
private:
    GMainLoop *loop{};
    bool terminate{};
    bool publish{};
    std::string sourceUrl;

    static gboolean sourceBus(GstBus *bus, GstMessage *msg, gpointer data);
    static void onPadAdded(GstElement *element, GstPad *pad, gpointer data);

public:
    explicit GStreamer(std::string url);

    int startSource();

    static int startPublish();
};


#endif //GSTREAMER_GSTREAMER_H
