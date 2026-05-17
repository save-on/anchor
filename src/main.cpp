#include "banner.h"
#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>
#include <vector>

struct ElementData {
    GstElement *audio_source, *video_source, *video_queue, *audio_queue, *audio_convert,
        *video_convert, *audio_encoder, *video_encoder, *mux, *sink;
};

struct PadData {
    GstPad *mux_audio, *mux_video, *queue_audio, *queue_video;
};

bool handleElementCheck(std::vector<GstElement *> elements) {
    for (int i = 0; i < elements.size(); ++i) {
        if (!elements[i]) {
            std::cerr << "error: " << elements[i] << " could not be created\n";
            return 1;
        }
    }
    return 0;
}

/*

                             --- PIPELINE VISUAL DRAFT ---
                 v4l2src__videoconvert__(nvh264enc)__queue__
                                                            \
                                                             --mp4mux__filesink
           autoaudiosrc__audioconvert__(lamemp3enc)__queue__/


gst-launch-1.0 -e \
mp4mux name=mux ! filesink location=video.mp4 \
v4l2src ! videoconvert ! nvh264enc ! queue ! mux. \
autoaudiosrc ! audioconvert ! lamemp3enc ! queue ! mux.
*/

int main(int argc, char *argv[]) {
    // handleBanner();

    ElementData          element;
    PadData              pad;
    GstBus              *bus;
    GstMessage          *msg;
    GstStateChangeReturn ret;
    GstElement          *pipeline;

    gst_init(&argc, &argv);

    pipeline              = gst_pipeline_new("anchor_pipeline");
    element.video_source  = gst_element_factory_make("v4l2src", "video_source");
    element.audio_source  = gst_element_factory_make("autoaudiosrc", "audio_source");
    element.video_convert = gst_element_factory_make("videoconvert", "video_convert");
    element.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    element.video_encoder = gst_element_factory_make("nvh264enc", "video_encoder");
    element.audio_encoder = gst_element_factory_make("lamemp3enc", "audio_encoder");
    element.video_queue   = gst_element_factory_make("queue", "video_queue");
    element.audio_queue   = gst_element_factory_make("queue", "audio_queue");
    element.mux           = gst_element_factory_make("mp4mux", "mux");
    element.sink          = gst_element_factory_make("filesink", "file_output");

    // clang-format off
    const std::vector<GstElement *> currentElements = {
        pipeline,
        element.video_source,
        element.audio_source,
        element.video_convert,
        element.audio_convert,
        element.video_encoder,
        element.audio_encoder,
        element.video_queue,
        element.audio_queue,
        element.mux,
        element.sink
    };
    // clang-format on

    if (handleElementCheck(currentElements))
        return 1;

    // configuration
    g_object_set(element.sink, "location", "../output/video.mp4", NULL);
    g_object_set(element.video_source, "do-timestamp", TRUE, NULL);

    gst_bin_add_many(GST_BIN(pipeline), element.video_source, element.video_convert,
                     element.video_encoder, element.video_queue, element.audio_source,
                     element.audio_convert, element.audio_encoder, element.audio_queue, element.mux,
                     element.sink, NULL);

    if (gst_element_link(element.mux, element.sink) != TRUE ||
        gst_element_link_many(element.video_source, element.video_convert, element.video_encoder,
                              element.video_queue, NULL) != TRUE ||
        gst_element_link_many(element.audio_source, element.audio_convert, element.audio_encoder,
                              element.audio_queue, NULL) != TRUE) {
        std::cerr << "elements could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }

    // handle mux pad request linking
    // create pads
    pad.mux_audio = gst_element_request_pad_simple(element.mux, "audio_%u");
    std::cout << "Obtained request pad " << gst_pad_get_name(pad.mux_audio) << " audio branch\n";
    pad.queue_audio = gst_element_get_static_pad(element.audio_queue, "src");
    pad.mux_video   = gst_element_request_pad_simple(element.mux, "video_%u");
    std::cout << "Obtained request pad " << gst_pad_get_name(pad.mux_video) << " video branch\n";
    pad.queue_video = gst_element_get_static_pad(element.video_queue, "src");

    // link pads
    if (gst_pad_link(pad.queue_audio, pad.mux_audio) != GST_PAD_LINK_OK ||
        gst_pad_link(pad.queue_video, pad.mux_video) != GST_PAD_LINK_OK) {
        std::cout << "mux could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }

    gst_object_unref(pad.queue_audio);
    gst_object_unref(pad.queue_video);
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cout << "could not change pipeline state to playing\n";
        gst_object_unref(pipeline);
        return 1;
    }

    sleep(10);

    // initiate the eos
    gst_element_send_event(pipeline, gst_event_new_eos());

    // wait until error or eos
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    gst_element_release_request_pad(element.mux, pad.mux_audio);
    gst_element_release_request_pad(element.mux, pad.mux_video);
    gst_object_unref(pad.mux_audio);
    gst_object_unref(pad.mux_video);

    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(pipeline);
    return 0;
}
