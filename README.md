# Sloongnet - Only implement business as server's framework 
Copyright 2015-2021 Sloong.com. All Rights Reserved

[![Build Status](https://drone.sloong.com:8000/api/badges/sloongnet/engine/status.svg)](https://drone.sloong.com:8000/sloongnet/engine)

***
# Language 
[简体中文](https://git.sloong.com:8000/sloongnet/engine/src/branch/develop/README_CN.md)

# Overview
This is a set of modern and high-performance server side that only needs to implement its own business.
Using this system framework, there is no need to care about other details that are not related to business. You can quickly implement various functions by combining the modular components that come with it.
And in the case that the default components can not meet, according to the rules after writing their own module components, can also be easily embedded in the system to achieve their own unique needs.

# Feature
* Support for writing business systems using scripting languages
* Communication protocol based on Protobuf
* Support for Docker deployment
* Automatic dynamic expansion according to load conditions
* Event-driven design
* Support message priority mode, high priority requests will be processed first
* Asynchronous communication mode, the messages sent and then received are not corresponding, with the message priority function, low priority time-consuming requests will not affect the processing and return of high priority messages


# History
[View](https://git.sloong.com:8000/public/sloongnet/src/master/ChangeLog.md)

# Contact US
If have any question, tell us.

* [Email](mailto:admin@sloong.com)

# Reference
use library in this project.

* [SloongLibrary](https://git.sloong.com/public/library)
* Openssl
* libfmt
* libspdlog
* libprotobuf