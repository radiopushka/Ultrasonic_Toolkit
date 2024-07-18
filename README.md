# Ultrasonic_Toolkit
a tool that allows you to modulate and demodulate ultrasonic sound, the realtime version only works on Linux, the file version works on any platform 
- file to file should work on mac.

you can run make to compile both tools

run "make futx" to only compile the file to file tool

run "make utx" to only compile the real-time tool (Linux only, requires Alsa)

allows you to create DSB(dual sideband non-suppressed carrier) or AM(amplitude) modulation on any audio frequency of the audio present in an audio file. designed for use with ultrasound.


dependencies:
-alsa (optional)
-math.h

a usefull program: https://github.com/radiopushka/SonicSDR
