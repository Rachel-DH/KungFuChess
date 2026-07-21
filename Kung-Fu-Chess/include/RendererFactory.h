#pragma once

#include <memory>

#include "IRenderer.h"

// Constructs the concrete renderer (defined in Kung-Fu-Chess.UI), so main.cpp can call it without including any OpenCV headers.
std::unique_ptr<IRenderer> make_renderer();
