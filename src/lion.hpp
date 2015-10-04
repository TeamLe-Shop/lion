#pragma once

#define LION_UNIX_DIRECTORY "/tmp"
#define LION_UNIX_NAMESPACE "test"
#define LION_LOG_LEVEL Debug

// C Headers

#include <sys/un.h>

// C++ Headers

#include <string>
#include <queue>

#include <json11.hpp>

namespace lion {

using Socket = int;
using UnixAddress = struct ::sockaddr_un;
using LionMessage = json11::Json;

typedef struct {
    int id;
    std::string name;
} LogLevel;

const LogLevel Debug   {0, "Debug"};
const LogLevel Log     {1, "Log"};
const LogLevel Warning {2, "Warning"};
const LogLevel Severe  {3, "Severe"};

/// The base class for LionClient and LionBroker.
///
/// Implements core data members & functionality, such as the egress queue,
/// socket, and a logging function.
class LionBase {
public:
    /// Logs a message with the following format: <m_name> [loglevel] message
    /// The message wont be logged if the LogLevel's numerical value is lower
    /// than that of LION_LOG_LEVEL.
    /// The lowest log level is Debug, followed by Log, followed by Warning,
    /// followed by Severe, which is the highest level.
    void log(LogLevel l, std::string message);

protected:
    Socket m_socket;
    std::string m_name;
    bool m_valid = false;
    std::queue<LionMessage> egress;
};

/// A LionClient, as the name suggests, is a UNIX domain client that connects to
/// a LionBroker, and communicates using a publish-subscribe system.
///
/// LionClient::connect() must be used to connect to the broker.
/// It is important to note that the Unix domain socket address of both this
/// LionClient and the broker will be prefixed with `LION_UNIX_DIRECTORY` and
/// `LION_UNIX_NAMESPACE`.
///
///     LionClient sock("example");
///     sock.connect("mybroker");
///
/// LionClients can then subscribe to message types it wishes to receive.
///
///     sock.subscribe("example.message-type");
///
/// Messages can then be published with a chosen message type.
/// The message content is (currently) encoded in JSON, but may be switched to
/// MessagePack in the future.
///
///     sock.send("example.my-message", {"content": "hello world!"});
///
/// Messages must contain 2 fields: a "type" field and an "entity" field.
/// The above example encoded in JSON is as follows:
///
///     { "type": "example.my-message", "entity": {"content: "hello world!"} }
///
class LionClient : public LionBase {
public:
    LionClient(std::string name);
    LionClient(std::string name, std::string broker_address);
    ~LionClient();

    /// Connect to the broker.
    ///
    /// This starts a local connection to the Unix socket address
    /// LION_UNIX_DIRECTORY + LION_UNIX_NAMESPACE + `broker`.
    bool connect(std::string broker_address);
};

/// A LionBroker receives messages published by LionClients and distributes them
/// to interested or subscribed LionClients.
class LionBroker : public LionBase {
public:
    LionBroker(std::string name);
    ~LionBroker();

    /// Attempt to accept connections and receive messages from connected
    /// LionClients.
    void cycle();
};
} // namespace component
