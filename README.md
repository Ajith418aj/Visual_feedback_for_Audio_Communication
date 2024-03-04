# Visual_feedback_for_audio_communication

This application is being developed to provide an intuitive visual feedback to the audio communication that is taking place. It calculates various network metrices like RTT, number of packets dropped
and number of packets received and many other things and provides an animation that illustrates the audio packets being transferred among the clients.

The data forwarder contains header files to parse json objects. This can be downloaded from https://github.com/nlohmann/json

After downloading, move the folder "json-develop" to the working directory

cd to json-develop directory and
run these commands one after another

cmake .

make

sudo make install
