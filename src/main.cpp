#include "banner.h"
#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[]) {
    handleBanner();
    GstElement          *pipeline, *source, *sink;
    GstBus              *bus;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    source   = gst_element_factory_make("videotestsrc", NULL);
    sink     = gst_element_factory_make("autovideosink", NULL);
    pipeline = gst_pipeline_new("testpipeline");

    if (!pipeline || !source || !sink) {
        std::cout << "not all elements could be created\n";
        return 1;
    }

    g_object_set(source, "pattern", 18, NULL);
    g_object_set(source, "background-color", "0x0000ff00", NULL);
    gst_bin_add_many((GstBin *)(pipeline), source, sink, NULL);
    if (gst_element_link(source, sink) != TRUE) {
        std::cout << "elements could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cout << "could not change pipeline state to playing\n";
        gst_object_unref(pipeline);
        return 1;
    }

    sleep(10);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    return 0;
}
