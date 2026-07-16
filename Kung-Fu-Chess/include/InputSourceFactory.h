#pragma once

#include <memory>

#include "IInputSource.h"

// Constructs the concrete input source. Defined in Kung-Fu-Chess.UI (a later
// step) — declared here so main.cpp can call it without including any
// OpenCV headers.
std::unique_ptr<IInputSource> make_input_source();
