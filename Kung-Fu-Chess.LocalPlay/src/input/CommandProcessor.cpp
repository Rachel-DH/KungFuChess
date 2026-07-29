#include "input/CommandProcessor.h"

#include <optional>
#include <vector>

#include "control/BoardMapper.h"
#include "control/Controller.h"
#include "input/Parser.h"

CommandProcessor::CommandProcessor(Controller& controller, std::ostream& out) : controller_(controller), out_(out) {}

void CommandProcessor::run_line(const std::string& line) {
    std::vector<std::string> tokens = Parser::tokenize(line);
    if (tokens.empty()) {
        return;
    }

    try {
        if (tokens[0] == "click" && tokens.size() == 3) {
            std::optional<Position> cell = BoardMapper::pixel_to_cell(
                std::stoi(tokens[1]), std::stoi(tokens[2]), controller_.width(), controller_.height());
            if (cell.has_value()) {
                controller_.click(*cell);
            } else {
                controller_.deselect();
            }
        } else if (tokens[0] == "jump" && tokens.size() == 3) {
            std::optional<Position> cell = BoardMapper::pixel_to_cell(
                std::stoi(tokens[1]), std::stoi(tokens[2]), controller_.width(), controller_.height());
            if (cell.has_value()) {
                controller_.jump(*cell);
            }
        } else if (tokens[0] == "wait" && tokens.size() == 2) {
            controller_.wait(std::stoi(tokens[1]));
        } else if (tokens[0] == "print" && tokens.size() == 2 && tokens[1] == "board") {
            controller_.print(out_);
        }
    } catch (const std::exception&) {
        // Malformed numeric arguments (e.g. "click a b") are ignored.
    }
}
