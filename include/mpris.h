#pragma once
#include <dbus.h>
#include <algorithm>
#include <map>

const std::map<std::string, std::string> MEDIA_PLAYER = {
    {"Mozilla Firefox", "firefox"},
    {"cmus", "cmus"}};

struct Metadata
{
    std::string player;
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
    std::string GetMediaPlayer();
    Connection *connection;
    Proxy proxy;
};