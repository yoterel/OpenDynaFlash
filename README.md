# OpenDynaFlash

This library provides a C++ wrapper and python bindings for the official DynaFlash V3 API, which is supplied with the DynaFlash v3 projector (and also in [this repository](https://github.com/yoterel/OpenDynaFlash/tree/main/official_API), for completeness).

There are 3 reasons you should be using this library rather than calling the API functions directly.

1) The official API is ill-documented, and many edge cases / pitfalls are not directly attended to. This can lead to wrong usage including distorted color space, not reaching maximum throughput and latency, memory leaks, and even system crashes and [BSOD](https://en.wikipedia.org/wiki/Blue_screen_of_death).
2) This library is thread-safe and allows for integration into multi-threaded programs, as opposed to the non-thread-safe original API.
3) The library is open-sourced, allowing for un-documented features/bugs in both the DynaFlash driver and hardware to be exposed discussed and possibly solved to the benefit of everyone, as opposed to personal attempts or solutions happening locally and repeatedly.

# Installation
## In a C++ Project
To use in your own project, you will need to do the following things:

- Add the source file [OpenDynaFlash.cpp](https://github.com/yoterel/OpenDynaFlash/blob/main/src/OpenDynaFlash.cpp) to your project.
- Include all the files under the [include](https://github.com/yoterel/OpenDynaFlash/tree/main/include) folder in your project.

- Include & link the Official API in your project (mandatory even if you don't use OpenDynaFlash)
  - Include [DynaFlash.h](https://github.com/yoterel/OpenDynaFlash/blob/main/official_API/include/DynaFlash.h).
  - Link the [official dll](https://github.com/yoterel/OpenDynaFlash/blob/main/official_API/lib/DynaFlash200.dll) to your project.

If you use CMAKE, you can follow what is done to compile [example.cpp](https://github.com/yoterel/OpenDynaFlash/blob/main/examples/example.cpp) (notice the CMAKE target "example").


## In Python
An official pip package is being prepared. In the meantime, if you manage to compile the CMAKE target OpenDynaFlash (requires VS2019 or above, Python 3.9 or above), then the python library will be automatically created. Note you will need to place the [official dll](https://github.com/yoterel/OpenDynaFlash/blob/main/official_API/lib/DynaFlash200.dll) in the same location to actually use it.

# Usage
See example.cpp or example.py for typical usecases.

# Contribution
You are free to open bug reports and feature requests in the Issues section, and to contribute and improve this library.
