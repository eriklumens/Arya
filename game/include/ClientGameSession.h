#pragma once

#include "Arya.h"
#include "Events.h"
#include "GameSession.h"

#include <vector>
using std::vector;

using Arya::Root;
using Arya::Scene;
using Arya::Object;
using Arya::Model;
using Arya::ModelManager;
using Arya::Camera;
using Arya::Texture;
using Arya::TextureManager;
using Arya::Shader;
using Arya::ShaderProgram;

class GameSessionInput;
class Faction;

struct CellList;

class ClientGameSession :
    public Arya::FrameListener,
    public EventHandler,
    public GameSession
{
    public:
        ClientGameSession();
        ~ClientGameSession();

        bool init();
        bool initShaders();
        bool initVertices();

        void rebuildCellList();

        Faction* getLocalFaction() const { return localFaction; } ;
        const vector<Faction*>& getFactions() const { return factions; }

        // FrameListener
        void onFrame(float elapsedTime);
        void onRender();

        void handleEvent(Packet& packet);
        CellList* unitCells;

        bool findPath(const vec2& start, const vec2& end, vector<vec2>& outNodes);
     private:
        GameSessionInput* input;
        Faction* localFaction;
        vector<Faction*> factions;
        vector<int> clients;

        ShaderProgram* decalProgram;
        GLuint decalVao;

        GLuint selectionDecalHandle;
		void initPathfinding();
		unsigned char* pathfindingMap;
};
