#include "../include/common/GameLogger.h"
#include "../include/Network.h"
#include "../include/Server.h"
#include "../include/Packet.h"
#include "../include/Events.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Exception.h"
#include "Poco/Net/NetException.h"
#include <vector>
#include "Arya.h"

using std::vector;
using namespace Poco;
using namespace Poco::Net;

class Connection
{
    public:
        Connection() : socket(new StreamSocket), bufferSizeTotal(8192)
        {
            connected = false;
            connecting = false;
            dataBuffer = new char[bufferSizeTotal+1];
            bytesReceived = 0;
            eventManager = 0;
        }

        ~Connection()
        {
            delete socket;
            delete[] dataBuffer;
        }

        void setEventManager(EventManager* handler)
        {
            eventManager = handler;
        }

        void connect(string host, int port)
        {
            if(connected || connecting) socket->close();
            socket->connectNB(SocketAddress(host,port));
            connected = false;
            connecting = true;
            GAME_LOG_INFO("Started connection to " << host);
        }

        void update()
        {
            if(connecting)
            {
                if(socket->poll(0, StreamSocket::SELECT_WRITE))
                {
                    GAME_LOG_INFO("Connected to server!");
                    connected = true;
                    connecting = false;
                }
            }
            else if(connected)
            {
                if(socket->poll(0, StreamSocket::SELECT_READ))
                {
                    int n = 0;
                    try
                    {
                        n = socket->receiveBytes(dataBuffer + bytesReceived, bufferSizeTotal - bytesReceived);
                    }
                    catch(TimeoutException& e)
                    {
                        GAME_LOG_WARNING("Timeout exception when reading socket! Msg: " << e.displayText());
                    }
                    catch(NetException& e)
                    {
                        GAME_LOG_WARNING("Net exception when reading socket Msg: " << e.displayText());
                    }

                    if(n <= 0)
                    {
                        GAME_LOG_INFO("Server closed connection");
                        socket->close();
                        connected = false;
                        return;
                    }
                    else
                    {
                        bytesReceived += n;

                        //Check if we received the packet header
                        //The while is because we often receive
                        //multiple packets in a single receive call
                        while(bytesReceived >= 12)
                        {
                            if( *(int*)dataBuffer != PACKETMAGICINT )
                            {
                                GAME_LOG_WARNING("Invalid packet header! Closing connection.");
                                socket->close();
                                connected = false;
                                return;
                            }
                            int packetSize = *(int*)(dataBuffer + 4); //this is including the header
                            if(packetSize > bufferSizeTotal)
                            {
                                if(packetSize > 5*1024*1024) //5 MB
                                {
                                    GAME_LOG_WARNING("Packet does not fit in buffer. Possible hack attempt. Removing client. Packet size = " << packetSize);
                                    socket->close();
                                    connected = false;
                                    return;
                                }
                                else
                                {
                                    //Packet is too large. Allocate bigger buffer.
                                    char* newBuffer = new char[packetSize+1];
                                    memmove(newBuffer, dataBuffer, bytesReceived);
                                    delete[] dataBuffer;
                                    dataBuffer = newBuffer;
                                    bufferSizeTotal = packetSize+1;
                                }
                                break;
                            }
                            if(bytesReceived >= packetSize)
                            {
                                handlePacket(dataBuffer, packetSize);
                                //if there was more data in the buffer, move it
                                //to the start of the buffer
                                int extraSize = bytesReceived - packetSize;
                                if(extraSize > 0)
                                    memmove(dataBuffer, dataBuffer + packetSize, extraSize);
                                bytesReceived = extraSize;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                else if(socket->poll(0, StreamSocket::SELECT_WRITE))
                {
                    for(vector<Packet*>::iterator pak = packets.begin(); pak != packets.end(); )
                    {
                        if((*pak)->markedForSend)
                        {
                            //TODO: send partial if whole packet is not possible
                            //save the amount of bytes that have already been send in Packet
                            sendPacket(*pak);
                            delete *pak;
                            pak = packets.erase(pak);
                            //We could break here so that packets are spreaded
                            //accross different frames
                            //break;
                        }
                        else
                        {
                            ++pak;
                        }
                    }
                }
            }
            else
            {
                if(!packets.empty())
                {
                    GAME_LOG_WARNING("There are packets in the send queue but there is no connection!");
                    while(!packets.empty())
                    {
                        delete packets.back();
                        packets.pop_back();
                    }
                }
            }
        }

        void sendPacket(Packet* packet)
        {
            int bytesSent = 0;
            int totalSize = packet->getSize();
            char* data = packet->getData();

            while(bytesSent < totalSize)
            {
                int n = 0;
                try
                {
                    n = socket->sendBytes(data + bytesSent, totalSize - bytesSent);
                }
                catch(TimeoutException& e)
                {
                    GAME_LOG_WARNING("Timeout exception when writing to socket! Msg: " << e.displayText());
                    break;
                }
                catch(NetException& e)
                {
                    GAME_LOG_WARNING("Net exception when writing to socket. Msg: " << e.displayText());
                    break;
                }
                if(n<=0)
                {
                    GAME_LOG_INFO("Server closed connection when writing to socket");
                    socket->close();
                    connected = false;
                    break;
                }
                else
                {
                    bytesSent += n;
                }
            }
        }

        void handlePacket(char* data, int packetSize)
        {
            if(!eventManager)
            {
                GAME_LOG_WARNING("Incoming packet can not be handled because no eventmanager is set");
                return;
            }
            Packet pak(data, packetSize);
            eventManager->handlePacket(pak);
        }

        bool connected;
        bool connecting;
        StreamSocket* const socket; //const so it can not be made zero

        vector<Packet*> packets; //outgoing packets

        EventManager* eventManager;

        int bufferSizeTotal;
        char* dataBuffer;
        int bytesReceived;
};

Network::Network()
{
    server = 0;
    lobbyConnection = new Connection();
    sessionConnection = new Connection();
    eventHandler = 0;
}

Network::~Network()
{
    if(server) delete server;
    delete lobbyConnection;
    delete sessionConnection;
}

void Network::setPacketHandler(EventManager* handler)
{
    lobbyConnection->setEventManager(handler);
    sessionConnection->setEventManager(handler);
}

void Network::startServer()
{
    if(server) delete server;
    server = new Server;
    server->runInThread();
}

void Network::runServer()
{
    if(server) delete server;
    server = new Server;
    server->run();
}

void Network::connectToLobbyServer(string host, int port)
{
    lobbyConnection->connect(host,port);
}

void Network::connectToSessionServer(string host, int port)
{
    sessionConnection->connect(host,port);
}

Packet* Network::createLobbyPacket(int id)
{
    Packet* pak = new Packet(id);
    lobbyConnection->packets.push_back(pak);
    return pak;
}

Packet* Network::createSessionPacket(int id)
{
    Packet* pak = new Packet(id);
    sessionConnection->packets.push_back(pak);
    return pak;
}

void Network::update()
{
    lobbyConnection->update();
    sessionConnection->update();
}
