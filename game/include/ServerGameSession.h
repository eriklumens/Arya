#pragma once
#include <vector>

using std::vector;

class Server;
class ServerClient;
class Packet;

class ServerGameSession
{
    public:
        ServerGameSession(Server* serv) : server(serv)
        {
            gameState = STATE_CREATED;
            idFactory = 1;
        }
        ~ServerGameSession()
        {
        }

        void addClient(ServerClient* client);
        void removeClient(ServerClient* client);

        void handlePacket(ServerClient* client, Packet& packet);

        int getNewId() { return idFactory++; }

        unsigned int getClientCount() const { return clientList.size(); }

        void sendToAllClients(Packet* pak);

        bool gameReadyToLoad() const;
        bool gameReadyToStart() const;
        void startLoading();
        void startGame();
    private:
        Server* const server;
        int idFactory;

        vector<ServerClient*> clientList;
        typedef vector<ServerClient*>::iterator clientIterator;

        //Game state
        //1. CREATED: clients start connecting
        //2. LOADING: server has sent GAME_READY
        //            server is sending unit lists
        //3. STARTED: the game timer started
        enum{ STATE_CREATED = 0,
            STATE_LOADING = 1,
            STATE_STARTED = 2
        } gameState;

};
