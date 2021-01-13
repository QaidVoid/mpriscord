#include <mpris.h>

Mpris::Mpris()
{
    Connection *conn = new Connection;
    connection = conn;
}

// Return all available media player in current session
std::vector<std::string> Mpris::GetAllMediaPlayer()
{
    std::vector<std::string> result;
    auto bus = connection->GetAllBus();

    std::copy_if(bus.begin(), bus.end(), std::back_inserter(result), [](std::string &str) {
        return str.rfind("org.mpris.MediaPlayer2", 0) != std::string::npos;
    });
    return result;
}

// Return currently playing media player
std::string Mpris::GetCurrentMediaPlayer()
{
    auto players = GetAllMediaPlayer();

    for (auto &player : players)
    {
        auto proxy = connection->WithProxy(player, "/org/mpris/MediaPlayer2");
        auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        char* res;

        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
        {
            DBusMessageIter arrayIter;
            dbus_message_iter_recurse(&iter, &arrayIter);

            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arrayIter))
            {
                dbus_message_iter_get_basic(&arrayIter, &res);
                std::string r(res);
                if (r == "Playing") {
                    return player;
                }
            }
        }
    }
    return "No Media Playing!";
}