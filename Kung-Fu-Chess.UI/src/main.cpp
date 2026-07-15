#include "img.h"

#include <exception>
#include <iostream>

int main()
{
    try {
        Img img;
        img.read("../ext_src/board.png", {800, 600}, true);
        img.show();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
