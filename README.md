# Arduino Assistant
This is a script I use at home to control my bedroom lights with Google Home, whilst showing the currently playing Spotify track.

Setup requires an MQTT server to be running and a Wi-Fi network for the Arduino to connect to (I used a Feather M0).

Switching lights on and off is done using the rc-switch library. I have a Status-branded plug that communicates on the 433MHz spectrum.
