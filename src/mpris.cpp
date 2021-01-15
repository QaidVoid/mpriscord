#include <mpris.h>

Mpris::Mpris()
{
    Connection *conn = new Connection;
    connection = conn;
}

std::string Mpris::GetPlayerIdentity()
{
    auto reply = proxy.GetProperty("org.mpris.MediaPlayer2", "Identity");
    char *res;

    DBusMessageIter iter;
    if (dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
    {
        dbus_message_unref(reply);
        DBusMessageIter sub;
        dbus_message_iter_recurse(&iter, &sub);

        if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&sub))
        {
            dbus_message_iter_get_basic(&sub, &res);
        }
    }
    return res;
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
    std::vector<std::string> active;

    for (auto &player : players)
    {
        if (player == "org.mpris.MediaPlayer2.spotify")
            continue;
        proxy = connection->WithProxy(player, "/org/mpris/MediaPlayer2");
        auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        char *res;

        DBusMessageIter iter;
        if (dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
        {
            dbus_message_unref(reply);
            DBusMessageIter sub;
            dbus_message_iter_recurse(&iter, &sub);

            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&sub))
            {
                dbus_message_iter_get_basic(&sub, &res);
                std::string r(res);
                if (r == "Playing")
                {
                    active.push_back(player);
                }
            }
        }
    }

    int idx = -1;
    std::string player = "UNDEFINED";
    if (active.size() > 0)
        player = active[0];

    auto itr = std::find_if(std::begin(active), std::end(active), [&](const std::string &v) {
        std::string fi = v;
        fi.erase(0, 23);
        std::string mp = fi.substr(0, fi.find("."));

        auto it = std::find(PLAYER_PRIORITY, std::end(PLAYER_PRIORITY), mp);
        if (it != std::end(PLAYER_PRIORITY))
        {
            if (idx == -1 || idx > std::distance(PLAYER_PRIORITY, it))
            {
                idx = std::distance(PLAYER_PRIORITY, it);
                player = v;
            }
        }
        return false;
    });
    if (player != "UNDEFINED")
        proxy = connection->WithProxy(player, "/org/mpris/MediaPlayer2"); // Set proxy for high priority player
    return player;
}

// Return metadata from current media
Metadata *Mpris::GetMetadata()
{
    Metadata *metadata = new Metadata;
    auto player = GetCurrentMediaPlayer();

    if (&proxy != nullptr && player != "UNDEFINED")
    {
        metadata->identity = GetPlayerIdentity();

        if (!MEDIA_PLAYER.count(metadata->identity))
        {
            metadata->player = metadata->identity;
        }
        else
        {
            metadata->player = MEDIA_PLAYER.at(metadata->identity);
        }

        auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "Metadata");
        char *res;

        DBusMessageIter iter;
        if (reply != nullptr && dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
        {
            dbus_message_unref(reply);
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
                                        std::string artists;
                                        if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&dictValue))
                                        {
                                            dbus_message_iter_get_basic(&dictValue, &res);
                                            artists = res;
                                        }
                                        else if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&dictValue))
                                        {
                                            DBusMessageIter dictInnerValue;
                                            dbus_message_iter_recurse(&dictValue, &dictInnerValue);
                                            do
                                            {
                                                dbus_message_iter_get_basic(&dictInnerValue, &res);
                                                if (artists.empty())
                                                    artists = res;
                                                else
                                                    artists.append(", ").append(res);
                                            } while (dbus_message_iter_next(&dictInnerValue));
                                        }
                                        metadata->artist = artists;
                                    }
                                    else if (key == "mpris:length")
                                    {
                                        dbus_message_iter_get_basic(&dictValue, &metadata->length);
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

int64_t Mpris::GetPosition()
{
    DBusMessageIter iter;
    auto reply = proxy.GetProperty("org.mpris.MediaPlayer2.Player", "Position");
    int64_t pos = 0;

    if (reply != nullptr && dbus_message_iter_init(reply, &iter) && DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&iter))
    {
        dbus_message_unref(reply);
        DBusMessageIter sub;
        dbus_message_iter_recurse(&iter, &sub);

        if (DBUS_TYPE_INT64 == dbus_message_iter_get_arg_type(&sub))
        {
            dbus_message_iter_get_basic(&sub, &pos);
        }
    }
    return pos;
}

std::ostream &operator<<(std::ostream &out, const Metadata &m)
{
    out << "Playing on " << m.identity << "\n"
        << "Title: " << m.title << "\n"
        << "Album: " << m.album << "\n"
        << "Artist: " << m.artist << "\n"
        << "Length: " << m.length;
    return out;
}