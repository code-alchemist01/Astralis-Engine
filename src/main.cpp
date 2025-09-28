#include "core/App.hpp"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        App app;
        return app.run(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred!" << std::endl;
        return -1;
    }
}