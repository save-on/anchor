#include "banner.h"
#include <array>
#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>

struct VideoElementData {
    GstElement *videorate, *queue;
};

struct AudioElementData {
    GstElement *convert, *queue, *resample;
};

struct PadData {
    GstPad *teeAudio, *teeVideo, *queueAudioHead, *queueAudioTail, *queueVideoHead, *queueVideoTail,
        *funnelAudio, *funnelVideo;
};

bool handleElementCheck(std::array<GstElement *, 10> elements) {
    for (int i = 0; i < elements.size(); ++i) {
        if (!elements[i]) {
            std::cerr << "error: " << elements[i] << " could not be created\n";
            return 1;
        }
    }
    return 0;
}

/*
    > Audio fix
    - seems like I'll have to build a multithreaded pipeline
    - we'll need a "tee" probably a demuxer (tee)
    - we'll want a queue to pause data stream until have enough bytes (queue)
    - audio will probably need a convert (audioconvert)
    - add a audio resampler (audioresample)
    - current video pipeline should be fine
    - in order to make sure audio and video are on the same with timestamps <--
    - need a funnel to bring pipeline back to filesink

                             --- PIPELINE VISUAL DRAFT ---
                        v4l2src__videoconvert__(nvh264enc)__
                                                            \
                                                             --mp4mux__filesink
                       alsasrc__audioconvert__(audioencoder)/


gst-launch-1.0 -e \
mp4mux name=mux ! filesink location=video.mp4 \
v4l2src ! videoconvert ! nvh264enc ! queue ! mux. \
alsasrc ! audioconvert ! lamemp3enc ! queue ! mux.

    (video)                             (audio)
    source  v4l2src                     source  alsasrc
            (x-raw)                             (x-raw)
            (mpeg)
            (mpegts)
            (x-av1)
            (x-bayer)
            (x-dv)
            (x-fwht)
            (x-h263)
            (x-h264)
            (x-h265)
            (x-pwc1)
            (x-pwc2)
            (x-sonix)
            (x-vp8)
            (x-vp9)
            (x-wmv)
    enc     nvh264enc                   enc      lamemp3enc
            (x-h264)                            (x-mpeg)
    mux     mp4mux                      mux     mp4mux
            (x-mpeg)                            (x-mpeg v: 1, 4)
            (x-divx)                            (x-ac3)
            (x-h264)                            (x-eac3)
            (x-h265)                            (x-alac)
            (x-h266)                            (x-opus)
            (x-mp4-part)
            (x-av1)
            (x-vp9)

*/

int main(int argc, char *argv[]) {
    // handleBanner();

    // GstBus              *bus;
    GstStateChangeReturn ret;
    GstElement          *pipeline, *source, *sink;

    gst_init(&argc, &argv);

    // clang-format off
    pipeline = gst_pipeline_new("anchor_pipeline");
                                                    // video/audio source
    source              = gst_element_factory_make("v4l2src", "VA_source");
    sink                = gst_element_factory_make("filesink", "file_output");

    const std::array<GstElement *, 10> currentElements = {
        pipeline,
        source,
        sink
    };
    // clang-format on

    if (handleElementCheck(currentElements)) {
        std::cout << "not all elements could be created\n";
        return 1;
    }

    // Configuration
    g_object_set(sink, "location", "../output/video.mp4", NULL);
    g_object_set(source, "do-timestamp", TRUE, NULL);

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cout << "could not change pipeline state to playing\n";
        gst_object_unref(pipeline);
        return 1;
    }

    sleep(10);

    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(pipeline);
    return 0;
}
