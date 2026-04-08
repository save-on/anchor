#include "banner.h"
#include <cstdlib>
#include <iostream>
#include <string>

// MVP?
// upon start it should start recording video and audio.
// - should chunk data like a buffer to a server
// upon stop it should handle data according to user settings

int main(int args, char *strArg[]) {
    if (args < 2 || args > 2) {
        std::cout << "Must have 1 argument" << '\n';
        return 1;
    }
    std::string argument{strArg[1]};
    handleBanner();

    std::string command{"ffmpeg -i " + argument + " -map 0 -c copy ../output/output.mp4"};

    const char *mpvCommand{command.c_str()};

    system(mpvCommand);

    std::string input{};

    std::cout << '\n' << "Monitoring has started.." << '\n';
    while (true) {

        std::getline(std::cin, input);
        if (input == "q") {
            std::cout << "signal terminated" << '\n';
            break;
        }
    }

    return 0;
}
