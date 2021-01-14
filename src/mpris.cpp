#include <mpris.h>
#include <unistd.h>

Mpris::Mpris()
{
    Connection *conn = new Connection;
    connection = conn;
}

std::string GetMediaPlayer(std::string &str)
{
    str.erase(0, 23);
    std::string player = str.substr(0, str.find("."));
    return player;
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
        proxy = connection->WithProxy(player, "/org/mpris/MediaPlayer2");
        auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        char *res;

        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
        {
            DBusMessageIter sub;
            dbus_message_iter_recurse(&iter, &sub);

            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&sub))
            {
                dbus_message_iter_get_basic(&sub, &res);
                std::string r(res);
                if (r == "Playing")
                {
                    return player;
                }
            }
        }
    }
    return "UNDEFINED";
}

// Return metadata from current media
Metadata *Mpris::GetMetadata()
{
    Metadata *metadata = new Metadata;
    auto player = GetCurrentMediaPlayer();

    if (&proxy != nullptr && player != "UNDEFINED")
    {
        metadata->player = GetMediaPlayer(player);
        auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "Metadata");
        char *res;
        std::vector<std::string> artists;

        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
        {
            DBusMessageIter sub;
            dbus_message_iter_recurse(&iter, &sub);

            if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&sub))
            {
                DBusMessageIter arrayIter;
                dbus_message_iter_recurse(&sub, &arrayIter);

                do
                {
                    if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayIter))
                    {
                        DBusMessageIter dictEntryIter;

                        dbus_message_iter_recurse(&arrayIter, &dictEntryIter);
                        bool is_key = true;
                        std::string key;

                        do
                        {
                            if (is_key)
                            {
                                if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&dictEntryIter))
                                {
                                    dbus_message_iter_get_basic(&dictEntryIter, &res);
                                    is_key = false;
                                    key = res;
                                }
                            }
                            else
                            {
                                DBusMessageIter dictValue;
                                if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&dictEntryIter))
                                {
                                    dbus_message_iter_recurse(&dictEntryIter, &dictValue);
                                    if (key == "xesam:title")
                                    {
                                        dbus_message_iter_get_basic(&dictValue, &res);
                                        metadata->title = res;
                                    }
                                    else if (key == "xesam:album")
                                    {
                                        dbus_message_iter_get_basic(&dictValue, &res);
                                        metadata->album = res;
                                    }
                                    else if (key == "xesam:artist")
                                    {
                                        DBusMessageIter dictInnerValue;
                                        dbus_message_iter_recurse(&dictValue, &dictInnerValue);
                                        dbus_message_iter_get_basic(&dictInnerValue, &res);
                                        metadata->artist = res;
                                    }
                                    else if (key == "mpris:length")
                                    {
                                        dbus_message_iter_get_basic(&dictValue, &metadata->length);
                                    }
                                    else if (key == "mpris:artUrl")
                                    {
                                        dbus_message_iter_get_basic(&dictValue, &res);
                                        metadata->artUrl = res;
                                    }
                                }
                            }
                        } while (dbus_message_iter_next(&dictEntryIter));
                    }
                } while (dbus_message_iter_next(&arrayIter));
            }
        }
        return metadata;
    }
    return nullptr;
}

std::ostream &operator<<(std::ostream &out, const Metadata &m)
{
    out << "Playing on " << m.player << "\n"
        << "Title: " << m.title << "\n"
        << "Album: " << m.album << "\n"
        << "Artist: " << m.artist << "\n"
        << "ArtUrl: " << m.artUrl << "\n"
        << "Length: " << m.length;
    return out;
}