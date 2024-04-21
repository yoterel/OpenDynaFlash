# OpenDynaFlash

This library provides a C++ wrapper and python bindings for the official DynaFlash V3 API, which is supplied with the DynaFlash v3 projector.

There are 3 reasons you should be using this library rather than calling the API functions directly.

1) The official API is ill-documented, and many edge cases / pitfalls are not directly attended to. This can lead to wrong usage including distorted color space, not reaching maximum throughput and latency, memory leaks, and even system crashes and [BSOD](https://en.wikipedia.org/wiki/Blue_screen_of_death).
2) This library is thread-safe and allows for integration into multi-threaded programs, as opposed to the non-thread-safe original API.
3) The library is open-sourced, allowing for un-documented features/bugs in both the DynaFlash driver and Hardware to be exposed, discussed and solved publicaly, as opposed to personal attempts or solutions.

# Installation
## In a C++ Project
To use in your own project, you will need to do the following things:

- Add to the source file [OpenDynaFlash.cpp](https://github.com/yoterel/OpenDynaFlash/blob/main/src/OpenDynaFlash.cpp).
- Include all the files under the [include](https://github.com/yoterel/OpenDynaFlash/tree/main/include) folder.

- Include & link the Official API in your project (mandatory even if you don't use OpenDynaFlash)
  - Include [DynaFlash.h](https://github.com/yoterel/OpenDynaFlash/blob/main/official_API/include/DynaFlash.h).
  - Link the [official dll](https://github.com/yoterel/OpenDynaFlash/blob/main/official_API/lib/DynaFlash200.dll) to your project.

If you use CMAKE, you can follow what is done to compile [example.cpp](https://github.com/yoterel/OpenDynaFlash/blob/main/examples/example.cpp) (Notice the CMAKE target "example").


## In Python
todo

# Usage
## In a C++ Project

## In Python
todo

# Contribution
You are free to open bug reports and feature requests in the Issues section, and to contribute and improve this library.
