#include <mpris.h>

int main() {
    Mpris mpris;
    auto players = mpris.GetAllMediaPlayer();
    for (auto &player: players) {
        std::cout << player << '\n';
    }
}