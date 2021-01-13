#include <mpris.h>

// Return all available media player in current session
std::vector<std::string> Mpris::GetAllMediaPlayer()
{
    std::vector<std::string> result;
    Connection *conn = new Connection;
    auto bus = conn->GetAllBus();

    std::copy_if(bus.begin(), bus.end(), std::back_inserter(result), [](std::string &str) {
        return str.rfind("org.mpris.MediaPlayer2", 0) != std::string::npos;
    });
    return result;
}