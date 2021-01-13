#include <dbus.h>

void abort_on_error(DBusError *error)
{
    if (dbus_error_is_set(error))
    {
        std::cout << error->message << "\n";
        exit(1);
    }
}

Connection::Connection()
{
    DBusError error;
    dbus_error_init(&error);
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    abort_on_error(&error);
    connection = conn;
}

Proxy<Connection*> *Connection::WithProxy(std::string bus_name, std::string path)
{
    auto *proxy = new Proxy<Connection*>;
    proxy->connection = this;
    proxy->bus_name = bus_name;
    proxy->path = path;
    return proxy;
}

template<typename T>
T Proxy<T>::method_call(std::string interface, std::string method)
{
    DBusError error;
    DBusMessage *msg, *reply;
    DBusMessageIter iter, sub;
    std::vector<std::string> result;
    char *res;

    dbus_error_init(&error);
    msg = dbus_message_new_method_call(bus_name.c_str(), path.c_str(), interface.c_str(), method.c_str());
    reply = dbus_connection_send_with_reply_and_block(
        connection->connection,
        msg,
        1000,
        &error);
    abort_on_error(&error);
    dbus_message_unref(msg);
    return reply;
}