#include "banner.h"
#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>

struct CustomData {
    GstElement *pipeline, *source, *sink, *convert;
};

int main(int argc, char *argv[]) {
    // handleBanner();

    CustomData           data;
    GstBus              *bus;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    /*
        attach a webcam source
        attach a screen sink
    */

    data.source   = gst_element_factory_make("v4l2src", NULL);
    data.sink     = gst_element_factory_make("filesink", NULL);
    data.pipeline = gst_pipeline_new("testpipeline");

    if (!data.pipeline || !data.source || !data.sink) {
        std::cout << "not all elements could be created\n";
        return 1;
    }

    gst_bin_add_many((GstBin *)(data.pipeline), data.source, data.sink, NULL);
    if (gst_element_link_many(data.source, data.sink, NULL) != TRUE) {
        std::cout << "elements could not be linked\n";
        gst_object_unref(data.pipeline);
        return 1;
    }
    g_object_set(data.sink, "location", "../output/video.mp4", NULL);
    g_object_set(data.source, "do-timestamp", TRUE, NULL);

    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cout << "could not change data.pipeline state to playing\n";
        gst_object_unref(data.pipeline);
        return 1;
    }

    sleep(10);

    gst_element_set_state(data.pipeline, GST_STATE_NULL);

    return 0;
}
