#include <iostream>

#include "OpenDynaFlash.h"
// used for loading images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// used to measure performance and demonstrate multi-threading
#include "timer.h"

void project_white_blocking(DynaFlashProjector *projector, int num_frames)
{
    int counter = 0;
    while (counter < num_frames)
    {
        projector->show(); // projector will show a white image
        counter += 1;
    }
}

void project_data_blocking(DynaFlashProjector *projector, uint8_t *frame, int num_frames)
{
    int counter = 0;
    while (counter < num_frames)
    {
        if (!projector->show_buffer(frame))
        {
            std::cout << "Should never happen." << counter << std::endl;
        }
        counter += 1;
    }
}

void project_data_non_blocking(DynaFlashProjector *projector, uint8_t *frame, int num_frames)
{
    int counter = 0;
    while (counter < num_frames)
    {
        if (!projector->show_buffer(frame, false))
        {
            std::cout << "Circular buffer was full for frame: " << counter << " ...so the frame was not enqueued. What do you want to do?" << std::endl;
            // ... continue program or do something about it
        }
        counter += 1;
    }
}
void project_directly(DynaFlashProjector *projector, uint8_t *frame, int num_frames)
{
    int counter = 0;
    while (counter < num_frames)
    {
        uint8_t *dyna_raw_buffer = projector->get_internal_buffer(); // get the internal buffer
        memcpy(dyna_raw_buffer, frame, 1024 * 768 * 3);              // copy the frame to the internal buffer
        projector->post_internal_buffer();                           // project the frame
    }
}

bool load_image(const std::string fileName, uint8_t *image_data)
{
    int imageWidth, imageHeight, imageBPP;
    image_data = stbi_load(fileName.c_str(), &imageWidth, &imageHeight, &imageBPP, 0);
    if (image_data == NULL || imageWidth != 1024 || imageHeight != 768 || imageBPP != 3)
    {
        std::cout << "Failed to load image." << std::endl;
        return false;
    }
    // lets flip the R and the B channels since DynaFlash works BGR
    for (int i = 0; i < 1024 * 768; i++)
    {
        uint8_t temp = image_data[i * 3];
        image_data[i * 3] = image_data[i * 3 + 2];
        image_data[i * 3 + 2] = temp;
    }
    return true;
}

int main(int argc, char *argv[])
{
    Timer timer; // used for perforamnce measurements
    std::string fileName = "../../resources/XGA_colorbars_vert.png";
    uint8_t *my_frame = nullptr;
    if (!load_image(fileName, my_frame))
        return -1;

    // projector is created on the heap, but could also be created on the stack if needed
    DynaFlashProjector *projector = new DynaFlashProjector(false, false, 20, true); // create projector with circular buffer of size of 20
    projector->init();                                                              // init projector

    // first let's project a single white frame.
    // the projector keeps projecting the last uploaded frame until a new one is uploaded, or until the projector is stopped
    timer.start();
    project_white_blocking(projector, 1);
    timer.stop();
    std::cout << "single white frame: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;

    // now lets project 1K frames as quickly as possible. The "producer" (our app) will send frames much faster than the "consumer" (the projector) can display them
    // if left to its own devices, the projector will work in a FIFO mode, and if its internal buffer is full, it will start to drop frames.
    // we can increase the hardware buffer size to some extent, but the projector will still drop frames if the producer is too fast in the steady state.
    // this is why we rather use our own circular buffer to manage the frames we send to the projector, which allows control over whether to drop frames or not.
    // this also allows multiple producers to project frames, and the projector will display them in the order they were received.
    // note that the circular buffer will start blocking once full. This ensures no frames will be dropped.
    timer.start();
    project_white_blocking(projector, 1000);
    timer.stop();
    std::cout << "white frame, 1000 frames: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;

    // lets do the same thing, but this time with our own data (the white image from before is created inside OpenDynaFlash).
    // using the show_buffer function will eventually do two types of copies internally:
    // the first is to copy the passed pointer to the buffer (i.e the address) to the circular buffer which is extremely fast.
    // the second copy happens when the projector thread dequeues this pointer: it will immediatly request the internal buffer from the projector and copy the data to it.
    // note 1: remember there is a big difference in copy speed if this data is located on the stack or on the heap.
    // note 2: the show_buffer function does not release the buffer. This is on us since we are the allocators.
    timer.start();
    project_data_blocking(projector, my_frame, 1000);
    timer.stop();
    std::cout << "blocking, 1000 frames: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;

    // what if we didn't want to block when the circular buffer is full? we can pass blocking = false to the show_buffer function.
    // this specific example below will just drop the frames when the buffer is full.
    timer.start();
    project_data_non_blocking(projector, my_frame, 1000);
    timer.stop();
    std::cout << "non-blocking, 1000 frames: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;

    // In some use cases, we do not want to use the safe circular buffer to project, but rather directly project the latest frame.
    // for this, we can use the show_buffer_internal function. Note it is not thread safe, so calling it from multiple threads will result in race conditions.
    // but for a single producer, this is ok to use. As explained above the projector might drop frames internally.
    timer.start();
    project_directly(projector, my_frame, 1000);
    timer.stop();
    std::cout << "non-safe, 1000 frames: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;
    // until now we have been assuming our application desires to project in FIFO mode (i.e. the order of frames is important and buffering is allowed)
    // what about LIFO mode? (i.e. our application wants to avoid buffering, or just project the latest frame as soon as possible, and drop the rest)
    // we can use a queue of size 1 to achieve this behavior
    // note that the projector will still drop frames if the producer is too fast
    delete projector;
    projector = new DynaFlashProjector(false, false, 1, true); // create projector with circular buffer of size of 1
    projector->init();
    timer.start();
    timer.stop();
    std::cout << "LIFO, 1000 frames: " << timer.getElapsedTimeInMilliSec() << " ms" << std::endl;

    // projectors destructor will call kill() which will gracefully close the projector and free the resources
    return 0;
}
