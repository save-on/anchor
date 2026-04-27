#include "banner.h"
#include <array>
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

struct PadData {
    GstPad *teeAudio, *teeVideo, *queueAudio, *queueVideo, *funnelAudio, *funnelVideo;
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
                     __queue__audioconvert__audioresample__
                    /                                      \
     v4l2src--tee---                                        ---funnel--filesink
                    \__queue____________________videorate__/
*/

int main(int argc, char *argv[]) {
    // handleBanner();

    VideoData videoData;
    AudioData audioData;
    PadData   pad;
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
    videoData.videorate = gst_element_factory_make("videorate", "video_rate");
    audioData.queue     = gst_element_factory_make("queue", "audio_queue");
    audioData.convert   = gst_element_factory_make("audioconvert", "audio_converter");
    audioData.resample  = gst_element_factory_make("audioresample", "audio_resampler");
    funnel              = gst_element_factory_make("funnel", "funnel");
    sink                = gst_element_factory_make("filesink", "file_output");

    const std::array<GstElement *, 10> currentElements = {
        pipeline,
        source,
        tee,
        videoData.queue,
        videoData.videorate,
        audioData.queue,
        audioData.convert,
        audioData.resample,
        funnel,
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

    // clang-format off
    gst_bin_add_many(
            (GstBin *)(pipeline), 
            source,
            tee,
            videoData.queue,
            videoData.videorate,
            audioData.queue,
            audioData.convert,
            audioData.resample,
            funnel,
            sink,
            NULL
            );

    if (
        gst_element_link_many(source, tee, NULL) != TRUE ||
        gst_element_link_many(videoData.queue, videoData.videorate, NULL) != TRUE ||
        gst_element_link_many(audioData.queue, audioData.convert, audioData.resample, NULL) != TRUE ||
        gst_element_link_many(funnel, sink, NULL)
    ) {
        std::cout << "elements could not be linked\n";
        gst_object_unref(pipeline);
        return 1;
    }
    // clang-format on

    // connect pads to elements
    pad.teeVideo = gst_element_request_pad_simple(tee, "pad_tee_video");
    std::cout << "video branch request received: " << gst_pad_get_name(pad.teeVideo);
    pad.queueVideo  = gst_element_get_static_pad(videoData.queue, NULL);
    pad.funnelVideo = gst_element_request_pad_simple(funnel, "pad_funnel_video");
    std::cout << "video merge request received: " << gst_pad_get_name(pad.funnelVideo);

    pad.teeAudio = gst_element_request_pad_simple(tee, "pad_tee_audio");
    std::cout << "audio branch request received: " << gst_pad_get_name(pad.teeAudio);
    pad.queueAudio  = gst_element_get_static_pad(audioData.queue, NULL);
    pad.funnelAudio = gst_element_request_pad_simple(funnel, "pad_funnel_audio");
    std::cout << "audio merge request received: " << gst_pad_get_name(pad.funnelAudio);

    // So gst_pad_link has to parameters a src pad and a sink pad.
    // i'm currently trying to link 3 pads on one branch.
    // tee's on request is src
    // funnel's is on sink

    // clang-format off
    /* Under Construction */
    // clang-format on

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
