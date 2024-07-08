#include <iostream>
#include <thread>
#include <chrono>

#include <SFML/Audio.hpp>
#include "util.h"

int main(int argc, char** argv) {

    // Ensure proper usage.
    if (argc != 2) {
        std::cerr << "usage: ./" << argv[0] << " <audio-filename>" << std::endl;
        return EXIT_FAILURE;
    }

    // Remember filename.
    std::string filename = argv[1];

    // Open audio file.
    sf::Music music;
    if (!music.openFromFile(filename)) {
        std::cerr << "couldn't open file " << filename << std::endl;
        return EXIT_FAILURE;
    }

    music.play();
    std::this_thread::sleep_for(std::chrono::microseconds(music.getDuration().asMicroseconds()));

    return EXIT_SUCCESS;
}
