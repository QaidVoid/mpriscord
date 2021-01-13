#include <dbus.h>
#include <algorithm>

class Mpris
{
public:
    Mpris();
    std::vector<std::string> GetAllMediaPlayer();
    std::string GetCurrentMediaPlayer();

private:
    Connection *connection;
};