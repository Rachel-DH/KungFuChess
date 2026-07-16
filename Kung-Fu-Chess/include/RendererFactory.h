#pragma once

#include <memory>

#include "IRenderer.h"

// Constructs the concrete renderer. Defined in Kung-Fu-Chess.UI (step 2) —
// declared here so main.cpp can call it without including any OpenCV headers.
std::unique_ptr<IRenderer> make_renderer();
