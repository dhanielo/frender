#!/bin/bash
qmake "WEBKIT_CONFIG -= use_glib use_gstreamer use_gstreamer010 use_native_fullscreen_video legacy_web_audio web_audio video gamepad" -o Makefile WebKit.pro && make
