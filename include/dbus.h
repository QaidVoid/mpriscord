#include <dbus/dbus.h>
#include <iostream>

class Connection
{
public:
    Connection();
    Proxy *WithProxy(std::string bus_name, std::string path);

private:
    DBusConnection *connection;
};

class Proxy
{
public:
    Connection *connection;
    std::string bus_name;
    std::string path;
};