#pragma once
#include <dbus.h>
#include <algorithm>

struct Metadata
{
    std::string player;
    std::string title;
    std::string album;
    std::string artist;
    std::string artUrl;
    int64_t length;
    friend std::ostream &operator<<(std::ostream &out, const Metadata &m);
};

class Mpris
{
public:
    Mpris();
    std::vector<std::string> GetAllMediaPlayer();
    std::string GetCurrentMediaPlayer();
    Metadata *GetMetadata();

private:
    Connection *connection;
    Proxy proxy;
};