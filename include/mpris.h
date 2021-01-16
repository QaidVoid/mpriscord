#pragma once
#include <dbus.h>
#include <algorithm>
#include <map>

// Identity Name of the player mapped to discord rich presence asset key
const std::map<std::string, std::string> MEDIA_PLAYER = {
    {"Mozilla Firefox", "firefox"},
    {"cmus", "cmus"},
    {"VLC media player", "vlc"}
};

// Media player priority
const std::string PLAYER_PRIORITY[] = {
    "cmus",
    "plasma-browser-integration",
    "vlc",
    "firefox"
};

struct Metadata
{
    std::string identity;
    std::string title;
    std::string album;
    std::string artist;
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
    int64_t GetPosition();

private:
    std::string GetPlayerIdentity();
    Connection *connection;
    Proxy proxy;
};