#!/bin/bash
adb install -r bin/HelloFlow-debug.apk
adb shell am start -n org.oxygine.HelloFlow/org.oxygine.HelloFlow.MainActivity