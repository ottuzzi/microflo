#ifndef MICROFLO_H
#define MICROFLO_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "components.h"
#include "commandformat.h"

class MicroFlo {
public:
    void doNothing() {;}
};

// Packet
// TODO: implement a proper variant type, or type erasure
enum Msg {
    MsgInvalid = -1,
    MsgSetup,
    MsgTick,
    MsgCharacter,
    MsgBoolean,
    MsgEvent // like a "bang" in other flow-based systems
};

class Packet {

public:
    Packet();
    Packet(bool b);
    Packet(char c);
    Packet(Msg m);

public:
    bool boolean;
    char buf;
    enum Msg msg;
};

// Network
const int MAX_NODES = 10;
const int MAX_MESSAGES = 100;
const int MAX_PORTS = 10;

class Component;

struct Message {
    Component *target;
    int targetPort;
    Packet pkg;
};

typedef void (*MessageSendNotification)(int, Message, Component *, int);
typedef void (*MessageDeliveryNotification)(int, Message);

class Network {
public:
    Network();

    int addNode(Component *node);
    void connect(Component *src, int srcPort, Component *target, int targetPort);
    void connect(int srcId, int srcPort, int targetId, int targetPort);

    void sendMessage(Component *target, int targetPort, Packet &pkg,
                     Component *sender=0, int senderPort=-1);

    void setNotifications(MessageSendNotification send, MessageDeliveryNotification deliver);

    void runSetup();
    void runTick();
private:
    void deliverMessages(int firstIndex, int lastIndex);
    void processMessages();

private:
    Component *nodes[MAX_NODES];
    int lastAddedNodeIndex;
    Message messages[MAX_MESSAGES];
    int messageWriteIndex;
    int messageReadIndex;
    MessageSendNotification messageSentNotify;
    MessageDeliveryNotification messageDeliveredNotify;
};

struct Connection {
    Component *target;
    int targetPort;
};

// Component
// TODO: multiple ports
// TODO: a way of declaring component introspection data. JSON embedded in comment?
class Component {
    friend class Network;
public:
    static Component *create(ComponentId id);
    virtual void process(Packet in, int port) = 0;
protected:
    void send(Packet out, int port=0);
private:
    void connect(int outPort, Component *target, int targetPort);
    void setNetwork(Network *net) { network = net; }
private:
    Connection connections[MAX_PORTS]; // one per output port
    Network *network;
};



// Graph format
// TODO: defined commands for observing graph changes
#include <stddef.h>

#define GRAPH_MAGIC 'u','C','/','F','l','o',
const size_t GRAPH_MAGIC_SIZE = 6;
const size_t GRAPH_CMD_SIZE = 1 + 5; // cmd + payload

class GraphStreamer {
public:
    GraphStreamer();
    void setNetwork(Network *net) { network = net; }
    void parseByte(char b);
private:
    enum State {
        Invalid = -1,
        ParseHeader,
        ParseCmd
    };

    Network *network;
    int currentByte;
    char buffer[GRAPH_CMD_SIZE];
    enum State state;
};

#ifdef ARDUINO
class Debugger {
public:
    static void setup(Network *network);
    static void printPacket(Packet *p);
    static void printSend(int index, Message m, Component *sender, int senderPort);
    static void printDeliver(int index, Message m);
};
#endif // ARDUINO

#endif // MICROFLO_H
