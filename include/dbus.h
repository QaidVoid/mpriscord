#include <dbus/dbus.h>
#include <iostream>
#include <vector>

class Connection;

class Proxy
{
public:
    DBusMessage *method_call(std::string interface, std::string method);
    Connection *connection;
    std::string bus_name;
    std::string path;
};

class Connection
{
public:
    Connection();
    std::vector<std::string> GetAllBus();
    Proxy WithProxy(std::string bus_name, std::string path);
    DBusConnection *connection;
};