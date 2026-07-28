#include "input/InputSourceFactory.h"

#include "input/opencv_input_source.h"

std::unique_ptr<IInputSource> make_input_source() {
    return std::make_unique<OpenCvInputSource>("Kung Fu Chess");
}
