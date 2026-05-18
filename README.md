# anchor

run this command
cmake -B ninja -G Ninja

## CLI Pipeline

gst-launch-1.0 -e \
mp4mux name=mux ! filesink location=video.mp4 \
v4l2src ! videoconvert ! nvh264enc ! queue ! mux. \
autoaudiosrc ! audioconvert ! lamemp3enc ! queue ! mux.
