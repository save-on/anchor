#include "banner.h"
#include <gst/gst.h>
#include <gst/gstmessage.h>
#include <iostream>
#include <unistd.h>

struct VideoData {
    GstElement *videorate, *queue;
};

struct AudioData {
    GstElement *convert, *queue, *resample;
};

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
                     __queue__audioconvert__audioresample__
                    /                                      \
     v4l2src--tee---                                        ---funnel--filesink
                    \__queue____________________videorate__/
*/

int main(int argc, char *argv[]) {
    // handleBanner();

    VideoData videoData;
    AudioData audioData;
    // GstBus              *bus;
    GstStateChangeReturn ret;
    GstElement          *pipeline, *tee, *source, *sink, *funnel;

    gst_init(&argc, &argv);

    // clang-format off
    pipeline = gst_pipeline_new("anchor_pipeline");
                                                    // video/audio source
    source              = gst_element_factory_make("v4l2src", "VA_source");
    tee                 = gst_element_factory_make("tee", "tee");
    videoData.queue     = gst_element_factory_make("queue", "video_queue");
    audioData.queue     = gst_element_factory_make("queue", "audio_queue");
    videoData.videorate = gst_element_factory_make("videorate", "video_rate");
    audioData.convert   = gst_element_factory_make("audioconvert", "audio_converter");
    audioData.resample  = gst_element_factory_make("audioresample", "audio_resampler");
    funnel              = gst_element_factory_make("funnel", "funnel");
    sink                = gst_element_factory_make("filesink", "file_output");
    // clang-format on

    if (!pipeline || !source || !videoData.videorate || !sink) {
        std::cout << "not all elements could be created\n";
        return 1;
    }

    // clang-format off
    gst_bin_add_many(
            (GstBin *)(pipeline), 
            source, 
            videoData.videorate, 
            sink,
            NULL
            );
    // clang-format on

    if (gst_element_link_many(source, videoData.videorate, sink, NULL) != TRUE) {
        std::cout << "elements could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }
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

    return 0;
}
