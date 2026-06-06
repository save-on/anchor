#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>
#include <vector>

/*
                          --- PIPELINE VISUAL DRAFT ---
        v4l2src__timeoverlay__videoconvert__nvh265enc__h265parse__queue__
                                                                         \
                                                                         --dashsink
                     autoaudiosrc__audioconvert__faac__aacparse__queue__/
*/

struct ElementData {
    GstElement *audio_source, *video_source, *time_overlay, *video_queue, *audio_queue,
        *audio_convert, *video_convert, *audio_encoder, *video_encoder, *audio_parse, *video_parse,
        *mux, *dash;
};

struct PadData {
    GstPad *dash_audio, *dash_video, *queue_audio, *queue_video;
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

int main(int argc, char *argv[]) {
    ElementData          element;
    PadData              pad;
    GstBus              *bus;
    GstMessage          *msg;
    GstStateChangeReturn ret;
    GstElement          *pipeline;

    gst_init(&argc, &argv);

    pipeline              = gst_pipeline_new("anchor_pipeline");
    element.video_source  = gst_element_factory_make("v4l2src", "video_source");
    element.time_overlay  = gst_element_factory_make("timeoverlay", "time_overlay");
    element.audio_source  = gst_element_factory_make("autoaudiosrc", "audio_source");
    element.video_convert = gst_element_factory_make("videoconvert", "video_convert");
    element.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    element.video_encoder = gst_element_factory_make("nvh265enc", "video_encoder");
    element.audio_encoder = gst_element_factory_make("faac", "audio_encoder");
    element.video_parse   = gst_element_factory_make("h265parse", "video_parser");
    element.audio_parse   = gst_element_factory_make("aacparse", "audio_parser");
    element.video_queue   = gst_element_factory_make("queue", "video_queue");
    element.audio_queue   = gst_element_factory_make("queue", "audio_queue");
    element.dash          = gst_element_factory_make("dashsink", "dash_sink");

    // clang-format off
    const std::vector<GstElement *> currentElements = {
        pipeline,
        element.video_source,
        element.time_overlay,
        element.audio_source,
        element.video_convert,
        element.audio_convert,
        element.video_encoder,
        element.audio_encoder,
        element.video_parse,
        element.audio_parse,
        element.video_queue,
        element.audio_queue,
        element.dash
    };
    // clang-format on

    if (handleElementCheck(currentElements))
        return 1;

    // configuration

    // element.dash configs to look at
    // mpd-filename
    // mpd-root-path
    // muxer
    g_object_set(element.dash, "dynamic", TRUE, NULL);
    g_object_set(element.video_source, "do-timestamp", TRUE, NULL);
    g_object_set(element.time_overlay, "halignment", 2, "valignment", 1, NULL);

    gst_bin_add_many(GST_BIN(pipeline), element.video_source, element.time_overlay,
                     element.video_convert, element.video_encoder, element.video_parse,
                     element.video_queue, element.audio_source, element.audio_convert,
                     element.audio_encoder, element.audio_parse, element.audio_queue, element.dash,
                     NULL);

    if (gst_element_link_many(element.video_source, element.time_overlay, element.video_convert,
                              element.video_encoder, element.video_parse, element.video_queue,
                              NULL) != TRUE ||
        gst_element_link_many(element.audio_source, element.audio_convert, element.audio_encoder,
                              element.audio_parse, element.audio_queue, NULL) != TRUE) {
        std::cerr << "elements could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }

    // handle mux pad request linking
    // create pads
    pad.dash_audio = gst_element_request_pad_simple(element.dash, "audio_%u");
    std::cout << "Obtained request pad " << gst_pad_get_name(pad.dash_audio) << " audio branch\n";
    pad.queue_audio = gst_element_get_static_pad(element.audio_queue, "src");
    pad.dash_video  = gst_element_request_pad_simple(element.dash, "video_%u");
    std::cout << "Obtained request pad " << gst_pad_get_name(pad.dash_video) << " video branch\n";
    pad.queue_video = gst_element_get_static_pad(element.video_queue, "src");

    // link pads
    if (gst_pad_link(pad.queue_audio, pad.dash_audio) != GST_PAD_LINK_OK ||
        gst_pad_link(pad.queue_video, pad.dash_video) != GST_PAD_LINK_OK) {
        std::cout << "dash could not be linked\n";
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

    gst_object_unref(pad.dash_audio);
    gst_object_unref(pad.dash_video);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    if (msg != NULL)
        gst_message_unref(msg);

    gst_object_unref(bus);
    gst_object_unref(pipeline);
    return 0;
}
