#include <discord.h>

void DiscordState::SetMetadata()
{
    Mpris *mpris = new Mpris;
    DBusConnection *conn;
    DBusError error;
    DBusMessage *msg;
    DBusMessageIter args;
    char *value;

    Connection *connect = new Connection;
    conn = connect->connection;
    metadata = mpris->GetMetadata();
    if (metadata != nullptr)
        position = mpris->GetPosition();
    SetActivity();

    // add a rule for which messages we want to see
    dbus_bus_add_match(conn, "type='signal',interface='org.freedesktop.DBus.Properties'", &error); // see signals from the given interface
    dbus_connection_flush(conn);

    // loop listening for signals being emmitted
    while (true)
    {
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);

        // loop again if we haven't read a message
        if (NULL == msg)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", "PropertiesChanged"))
        {
            // read the parameters
            dbus_message_iter_init(msg, &args);

            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args))
            {
                dbus_message_iter_get_basic(&args, &value);
                if (std::string(value) == "org.mpris.MediaPlayer2.Player" && dbus_message_iter_next(&args))
                {
                    if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&args))
                    {
                        DBusMessageIter arrayIter;
                        dbus_message_iter_recurse(&args, &arrayIter);
                        do
                        {
                            if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayIter))
                            {
                                DBusMessageIter dictEntryIter;
                                dbus_message_iter_recurse(&arrayIter, &dictEntryIter);

                                if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&dictEntryIter))
                                {
                                    dbus_message_iter_get_basic(&dictEntryIter, &value);
                                    if (std::string(value) == "Metadata")
                                    {
                                        metadata = mpris->GetMetadata();
                                    }
                                    else if (std::string(value) == "PlaybackStatus")
                                    {
                                        if (dbus_message_iter_next(&dictEntryIter))
                                        {
                                            DBusMessageIter entry;
                                            dbus_message_iter_recurse(&dictEntryIter, &entry);
                                            dbus_message_iter_get_basic(&entry, &value);
                                            if (std::string(value) == "Stopped")
                                                metadata = nullptr;
                                            else
                                                metadata = mpris->GetMetadata();
                                        }
                                    }
                                }
                            }
                        } while (dbus_message_iter_next(&arrayIter));
                    }
                    if (metadata != nullptr)
                        position = mpris->GetPosition();
                    SetActivity();
                }
            }
        }

        // free the message
        dbus_message_unref(msg);
    }
}

void DiscordState::SetActivity()
{
    if (metadata != nullptr)
    {
        if (core == nullptr)
        {
            discord::Core *corr{};
            auto result = discord::Core::Create(CLIENT, DiscordCreateFlags_Default, &corr);
            core.reset(corr);
            if (!core)
            {
                std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                          << ")\n";
                std::exit(1);
            }

            core->SetLogHook(
                discord::LogLevel::Debug, [](discord::LogLevel level, const char *message) {
                    std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
                });
        }
        const char *player = metadata->player.c_str();
        std::string title = metadata->title;
        std::string artist = metadata->artist;

        discord::Activity activity{};

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto micro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        discord::Timestamp ts = ((micro - position) / 1e3);

        activity.SetType(discord::ActivityType::Listening);
        activity.SetDetails(title.c_str());
        activity.SetState(artist.c_str());
        activity.GetAssets().SetLargeImage(player);
        activity.GetAssets().SetLargeText(player);
        activity.GetTimestamps().SetStart(ts);
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
        isActive = true;
    }
    else if (isActive)
    {
        core = nullptr;
        // core->ActivityManager().ClearActivity([](discord::Result result) {});
        isActive = false;
    }
}
