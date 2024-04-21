#ifndef DYNAFLASH_H
#define DYNAFLASH_H
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <vector>
#include "readerwritercircularbuffer.h"
#include <thread>
#include "DynaFlash.h"
#ifdef PYTHON_BINDINGS_BUILD
#include <nanobind/ndarray.h>
#include <nanobind/nanobind.h>
namespace nb = nanobind;
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

class DynaFlashProjector
{
public:
    DynaFlashProjector(bool flip_ver = false, bool flip_hor = false, int queue_length = 20, bool verbose = false) : m_projector_queue(queue_length),
                                                                                                                    m_flip_ver(flip_ver),
                                                                                                                    m_flip_hor(flip_hor),
                                                                                                                    m_verbose(verbose){};
    ~DynaFlashProjector()
    {
        gracefully_close();
    };
    bool init();
    uint8_t *get_buffer();
    void show();
    void show_buffer(uint8_t *buffer);
    void show_buffer_internal(uint8_t *buffer);
    void kill() { gracefully_close(); };
    bool is_initialized() { return m_initialized; };
    void gracefully_close();
    std::size_t get_queue_size() { return m_projector_queue.size_approx(); };

#ifdef PYTHON_BINDINGS_BUILD
    void project(nb::ndarray<uint8_t, nb::shape<768, 1024, 3>,
                             nb::c_contig, nb::device::cpu>
                     data)
    {
        show_buffer(data.data());
    };
#endif
private:
    void print_version();
    void print_led_values();
    void set_led_values();
    const int m_frame_height = 768;
    const int m_frame_width = 1024;
    bool m_verbose = false;
    bool m_initialized = false;
    bool m_flip_ver = false;
    bool m_flip_hor = false;
    bool m_close_signal = false;
    int m_board_index = 0;
    float m_frame_rate = 946.0f; // max: 946.0f
    int m_bit_depth = 8;
    int m_alloc_frame_buffer = 16;
    // int allloc_src_frame_buffer=2000;
    ILLUMINANCE_MODE m_illum_mode = HIGH_MODE;
    int m_frame_mode = FRAME_MODE_RGB;
    int m_frame_size = FRAME_BUF_SIZE_24BIT;
    // char *pFrameData = NULL;
    // char *pFrameData[2] = { NULL };
    unsigned long m_nFrameCnt = 0;
    unsigned long m_nGetFrameCnt = 0;
    char *pBuf = NULL;
    CDynaFlash *pDynaFlash = NULL;
    DYNAFLASH_STATUS m_DynaFlashStatus;
    moodycamel::BlockingReaderWriterCircularBuffer<uint8_t *> m_projector_queue;
    std::thread m_projector_thread;
};

#ifdef PYTHON_BINDINGS_BUILD
NB_MODULE(dynaflash, m)
{
    nb::class_<DynaFlashProjector>(m, "projector")
        .def(nb::init<bool, bool, int, bool>(), nb::arg("flip_ver") = false,
             nb::arg("flip_hor") = false,
             nb::arg("queue_length") = 20,
             nb::arg("verbose") = false,
             "a class to control a dynaflash projector")
        .def("init", &DynaFlashProjector::init, "initializes the projector")
        .def("is_initialized", &DynaFlashProjector::is_initialized, "returns true if the projector is initialized")
        .def("kill", &DynaFlashProjector::kill, "frees the internal projector resources")
        .def("project_white", &DynaFlashProjector::show, "projects a white image")
        .def("project", &DynaFlashProjector::project, nb::arg("data"), "projects a numpy array of type uint8, with shape (height, width, 3), channels order is BGR");
}
#endif
#endif DYNAFLASH_H