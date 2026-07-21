#pragma once
#include <iostream>
#include <string>
class Controller;

// Adapter: parses one line of the stdin text protocol into a click/jump/wait/print Controller command; malformed or unknown lines are silently ignored, and "print board" writes to out_.
class CommandProcessor {
public:
    CommandProcessor(Controller& controller, std::ostream& out);
    void run_line(const std::string& line);
private:
    Controller& controller_;
    std::ostream& out_;
};
