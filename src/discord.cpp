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
            if (dbus_message_iter_init(msg, &args) && DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args))
            {
                dbus_message_iter_get_basic(&args, &value);
                metadata = mpris->GetMetadata();
                SetActivity();
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
        const char *title = metadata->title.c_str();
        std::string artist = std::string("by ").append(metadata->artist);
        std::string album = std::string("Album: ").append(metadata->album);

        discord::Activity activity{};
        activity.SetType(discord::ActivityType::Listening);
        activity.SetDetails(title);
        activity.SetState(artist.c_str());
        activity.GetAssets().SetLargeImage(player);
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
