#include "lion.hpp"

// C Headers

#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

// C++ Headers

#include <format.h>

namespace lion {

// LionBase

void LionBase::log(LogLevel l, std::string message)
{
    if (l.id >= LION_LOG_LEVEL.id) {
        fmt::print("<{}> [{}] {}\n", m_name, l.name, message);
    }
}

// LionClient

LionClient::LionClient(std::string name)
{
    m_name = name;

    log(Debug, fmt::format("New LionClient instance created with name '{}'",
                           name));
}

LionClient::LionClient(std::string name, std::string broker_address)
    : LionClient(name)
{
    m_valid = connect(broker_address);
}

bool LionClient::connect(std::string broker_address)
{
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (m_socket < 0) {
       log(Severe, fmt::format("Socket creation failed: {}", strerror(errno)));
       return false;
    }

    UnixAddress address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path,
           fmt::format("{}/{}.{}", LION_UNIX_DIRECTORY, LION_UNIX_NAMESPACE,
                       broker_address).c_str());

    int address_length = strlen(address.sun_path) + sizeof(address.sun_family);

    if (::connect(m_socket, (struct sockaddr *)&address, address_length) < 0) {
        log(Severe, fmt::format("Could not connect to UNIX domain socket {}:"
                                " {}", address.sun_path, strerror(errno)));
        return false;
    }

    fcntl(m_socket, F_SETFL, O_NONBLOCK);

    log(Log, fmt::format("Connected to broker at {}", address.sun_path));

    return true;
}

LionClient::~LionClient()
{
    close(m_socket);
    log(Debug, "Closed socket");
}

// LionBroker

LionBroker::LionBroker(std::string name)
{
    m_name = name;

    log(Debug, fmt::format("New LionBroker instance created with name '{}'",
                           name));

    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (m_socket < 0) {
        log(Severe, fmt::format("Socket creation failed: {}", strerror(errno)));
        return;
    }

    UnixAddress address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path,
           fmt::format("{}/{}.{}", LION_UNIX_DIRECTORY, LION_UNIX_NAMESPACE,
                       name).c_str());

    int address_length = strlen(address.sun_path) + sizeof(address.sun_family);

    if (bind(m_socket, (struct sockaddr *)&address, address_length) < 0) {
        log(Severe, fmt::format("Failed to bind to UNIX domain address {}:"
                                " {}", address.sun_path, strerror(errno)));
        return;
    }

    listen(m_socket, SOMAXCONN);

}

void LionBroker::cycle()
{

}

} // namespace component
