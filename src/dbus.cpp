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

Proxy *Connection::WithProxy(std::string bus_name, std::string path)
{
    Proxy *proxy = new Proxy;
    proxy->connection = this;
    proxy->bus_name = bus_name;
    proxy->path = path;
    return proxy;
}