# dynaflash

This library provides a C++ wrapper and python bindings around the official DynaFlash V3 API, which is supplied with the DynaFlash v3 projector.

There are 3 reasons you should be using this library rather than calling the API functions directly.

1) The official API is ill-documented, and many edge cases / pitfalls are not directly attended to. This can lead to wrong usage including distorted color space, not reaching maximum throughput and latency, memory leaks, and even system crashes and [BSOD](https://en.wikipedia.org/wiki/Blue_screen_of_death).
2) This library is thread-safe and allows for integration into multi-threaded programs, as opposed to the non-thread-safe original API.
3) The library is open-sourced, allowing for un-documented features/bugs in both the DynaFlash driver and Hardware to be exposed, discussed and solved publicaly, as opposed to personal attempts or solutions.

# Installation
todo

# DynaFlash V3 API
todo

# Contribution
You are free to open bug reports and feature requests in the Issues section, and to contribute and improve this library.
