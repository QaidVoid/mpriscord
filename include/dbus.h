#include <dbus/dbus.h>
#include <iostream>
#include <vector>

class Connection;

template <typename T>
class Proxy
{
public:
    T method_call(std::string interface, std::string method);
    Connection *connection;
    std::string bus_name;
    std::string path;
};

class Connection
{
public:
    Connection();
    Proxy<Connection *> *WithProxy(std::string bus_name, std::string path);
    DBusConnection *connection;
};