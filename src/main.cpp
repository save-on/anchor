#include "banner.h"
#include <gst/gst.h>
#include <iostream>
#include <string.h>

// MVP?
// upon start it should start recording video and audio.
// - should chunk data like a buffer to a server
// upon stop it should handle data according to user settings

// - input set up figure out how to get it to do livestream

struct CustomData {
    gboolean    is_live{};
    GstElement *pipeline{};
    GMainLoop  *loop{};
};

static void cb_message(GstBus *bus, GstMessage *msg, CustomData *data) {
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {

        GError *err;
        gchar  *debug;

        gst_message_parse_error(msg, &err, &debug);
        std::cerr << "Error: " << err->message << '\n';

        /* remember to free */
        g_error_free(err);
        g_free(debug);

        gst_element_set_state(data->pipeline, GST_STATE_READY);
        g_main_loop_quit(data->loop);
        break;
    }
    case GST_MESSAGE_EOS: {
        /* end-of-stream */
        gst_element_set_state(data->pipeline, GST_STATE_READY);
        g_main_loop_quit(data->loop);
        break;
    }
    case GST_MESSAGE_BUFFERING: {
        /* if the stream is live, ignore buffering */
        gint percent{0};
        if (data->is_live)
            break;
        gst_message_parse_buffering(msg, &percent);
        /* wait until the streams buffering has completed before playing */
        std::cout << "Buffering.. " << percent << "%\n";
        percent < 100 ? gst_element_set_state(data->pipeline, GST_STATE_PAUSED)
                      : gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
        break;
    }
    case GST_MESSAGE_CLOCK_LOST:
        gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
        gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
        break;
    default:
        // handle message
        break;
    }
}

int main(int argc, char *argv[]) {
    handleBanner();
    // if (argc < 3 || argc > 3) {
    //     std::cout << "Must have 2 argument" << '\n';
    //     return 1;
    // }

    GstElement          *pipeline;
    GstBus              *bus;
    GstStateChangeReturn ret;
    GMainLoop           *main_loop;
    CustomData           data;

    // initialize gstreamer
    gst_init(&argc, &argv);

    memset(&data, 0, sizeof(data));

    // build the pipeline
    pipeline = gst_parse_launch(
        "playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);
    bus = gst_element_get_bus(pipeline);

    // Start Playing
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cout << ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
        data.is_live = TRUE;
    }

    main_loop     = g_main_loop_new(NULL, FALSE);
    data.loop     = main_loop;
    data.pipeline = pipeline;

    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(cb_message), &data);
    g_main_loop_run(main_loop);

    g_main_loop_unref(main_loop);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    // std::cout << '\n' << "Monitoring has started.." << '\n';
    // std::string input{};

    // while (true) {
    //     std::getline(std::cin, input);
    //     if (input == "q") {
    //         std::cout << "signal terminated" << '\n';
    //         break;
    //     }
    // }

    return 0;
}
