#pragma once
#include "common/Singleton.h"
#include "Files.h"
#include "Interface.h"
#include <string>
#include <sstream>
#include <map>
using std::string;
using std::map;

namespace Arya
{
	class SettingsManager;
    enum ValueType
    {
        TYPE_UNKNOWN = 0,
        TYPE_STRING,
        TYPE_INTEGER,
        TYPE_FLOAT,
        TYPE_BOOL
    };
    struct cvar
    {
        ValueType type;
        string value;
        cvar(ValueType t, string v) : type(t), value(v) {};
        ~cvar(){};
        int getInt(){int ret; std::istringstream(value) >> ret; return ret;}
        float getFloat(){float ret; std::istringstream(value) >> ret; return ret;}
        bool getBool()
        {
            if(value == "true" || value == "True" || value == "TRUE" || value == "1") return true;
            if(value == "false" || value == "False" || value == "FALSE" || value == "0") return false;
            else return false;
        }
    };

    class Config : public Singleton<Config>, public CommandListener
    {
        public:
            Config();
            ~Config();
            bool init();
            void cleanup();
            void editConfigFile(string edit);
            cvar* getCvar(string name);
            int getCvarInt(string name);
            float getCvarFloat(string name);
            string getCvarString(string name);
            bool getCvarBool(string name);
            void setCvarWithoutSave(string name, string value, ValueType type = TYPE_STRING);
            void setCvar(string name, string value, ValueType type = TYPE_STRING);
            void setCvarInt(string name, int value);
            void setCvarFloat(string name, float value);
            void setCvarBool(string name, bool value);
            void setConfigFile(File* file);
			SettingsManager* getSettingsManager() const {return settingsManager;};
			void setSettingsManager(SettingsManager* _settingsManager){settingsManager = _settingsManager;};
			typedef map<string,cvar> cvarContainer;
			cvarContainer getCvarList(){return cvarList;};

        private:
            cvarContainer cvarList;
            bool loadConfigFileAfterRootInit(string configFileName);
            void updateConfigFile();
            File* configFile;
            bool loadConfigFile(string configFileName);
			bool handleCommand(string command);
			SettingsManager* settingsManager;
    };

	class SettingsManager : public ButtonDelegate
	{
		public:
			SettingsManager();
			~SettingsManager();
			bool init();
			void cleanup();
			void changeSetting(string setting, string value);
			void makeMenuActive(Window* window);
			void makeMenuInactive(Window* window);
			Window* getSettingsMenuWindow(){return settingsMenuWindow;};
			Window* getControlsMenuWindow(){return controlsMenuWindow;};
			Window* getGraphicsMenuWindow(){return graphicsMenuWindow;};

		private:
			void initSettingsMenu();

			Window* settingsMenuWindow;
			Window* controlsMenuWindow;
			Window* graphicsMenuWindow;

			typedef map<string,string> nameContainer;
			nameContainer nameList;
			void buttonClicked(Arya::Button* sender);
	};
}
