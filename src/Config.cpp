#include <sstream>
#include "common/Listeners.h"
#include "common/Logger.h"
#include "Config.h"
#include "Commands.h"
#include "Textures.h"
#include "Fonts.h"
using std::stringstream;
using std::fstream;
namespace Arya
{
    template<> Config* Singleton<Config>::singleton = 0;

    Config::Config()
    {
        configFile = 0;
    };

    Config::~Config()
    {
		CommandHandler::shared().removeCommandListener(this);
        cleanup();
    }

    bool Config::init()
    {
		CommandHandler::shared().addCommandListener("set", this);
		CommandHandler::shared().addCommandListener("get", this);
		CommandHandler::shared().addCommandListener("getVarValue", this);
		CommandHandler::shared().addCommandListener("setVarValue", this);

        setCvarWithoutSave("fullscreen", "false", TYPE_BOOL);
        setCvarWithoutSave("serveraddress", "localhost", TYPE_STRING);
		
		setCvarWithoutSave("goingforwardgame","W", TYPE_STRING);
		setCvarWithoutSave("goingbackwardgame","S", TYPE_STRING);
		setCvarWithoutSave("rotatingleftgame","Q", TYPE_STRING);
		setCvarWithoutSave("rotatingrightgame","E", TYPE_STRING);
		setCvarWithoutSave("goingleftgame","A", TYPE_STRING);
		setCvarWithoutSave("goingrightgame","D", TYPE_STRING);
		setCvarWithoutSave("goingdowngame","Z", TYPE_STRING);
		setCvarWithoutSave("goingupgame","X", TYPE_STRING);
		setCvarWithoutSave("synchronizegame","!", TYPE_STRING);

        loadConfigFile("config.txt");
        return true;
    }

    void Config::cleanup()
    {
        if(configFile != 0) FileSystem::shared().releaseFile(configFile);
        configFile = 0;
    }

    bool Config::loadConfigFileAfterRootInit(string configFileName)
    {
        configFile = FileSystem::shared().getFile(configFileName);
        if(configFile == 0) return false;
        std::stringstream fileStream(configFile->getData());
        string line;
        while(true)
        {
            getline(fileStream,line);
            if(!fileStream.good()) break;
            if(line.empty()) continue;
            if(line.substr(0,3) == "set") continue;
            if(line[0] == '#') continue;
        }
        return true;
    }
    void Config::updateConfigFile()
    {
        if(configFile != 0) FileSystem::shared().releaseFile(configFile);
        loadConfigFileAfterRootInit("config.txt");
        return;
    }

    void Config::editConfigFile(string edit)
    {
        if(!configFile) return;
        bool flag = false;
        std::ofstream outputFile((FileSystem::shared().getApplicationPath() + "config.txt").c_str());
        if(!outputFile.is_open()) 
        {
            LOG_WARNING("Could not open output config file!");
            return;
        }
        string editCommand = edit.substr(0, edit.find_first_of(" "));
        std::stringstream fileStream(configFile->getData());
        string line;
        while(true)
        {
            getline(fileStream,line);
            if(!fileStream.good()) break;
            if(line.empty())
            {
                outputFile << line << std::endl;
                continue;
            }
            else
            {
                string lineCommand = line.substr(0, edit.find_first_of(" "));
                if(lineCommand == editCommand)
                {
                    outputFile << edit << std::endl;
                    flag = true;
                }
                else
                {
                    outputFile << line << std::endl;
                }
            }
        }
        if(flag == false) outputFile << edit << std::endl;
        outputFile.close();
        updateConfigFile();
    }

    cvar * Config::getCvar(string name)
    {
        cvarContainer::iterator iter = cvarList.find(name);
        if(iter == cvarList.end()) return 0;
        else return &(iter->second);
    }
    int Config::getCvarInt(string name)
    {
        cvar* var = getCvar(name);
        if(var) return var->getInt();
        else
        {
            LOG_WARNING("Var " << name << " is not of this type!");
            return 0;
        }
    }
    float Config::getCvarFloat(string name)
    {
       cvar* var = getCvar(name);
       if(var) return var->getFloat();
       else
       {
           LOG_WARNING("Var " << name << " is not of this type!");
           return 0.0f;
       }
    }
    string Config::getCvarString(string name)
    {
        cvar* var = getCvar(name);
        if(var) return var->value;
        else
        {
            LOG_WARNING("Var " << name << " is not of this type!");
            return "";
        }
    }
    bool Config::getCvarBool(string name)
    {
        cvar* var = getCvar(name);
        if(var)
        {
            if(var->type != TYPE_BOOL)
                LOG_WARNING("Config var " << name << " is not a bool");
            return var->getBool();
        }
        else
        {
            LOG_WARNING("Config var " << name << " not found");
            return false;
        }
    }
    void Config::setCvarWithoutSave(string name, string value, ValueType type)
    {
        if(name.empty()) return;
        cvar* var = getCvar(name);
        if(var)
        {
            var->value = value;
            var->type = type;
        }
        else
        {
            cvarList.insert(cvarContainer::value_type(name, cvar(type, value)));
        }
    }
    void Config::setCvar(string name, string value, ValueType type)
    {
        if(name.empty()) return;
        cvar* var = getCvar(name);
        if(var)
        {
            var->value = value;
            var->type = type;
        }
        else
        {
            cvarList.insert(cvarContainer::value_type(name, cvar(type, value)));
            var = getCvar(name);
        }
        if(var == 0)
        {
            LOG_ERROR("Could not add new cvar '" << name << "' to list");
        }
        if(!configFile)
        {
            LOG_WARNING("configFile pointer is 0!");
            return;
        }
        bool flag = false;
        std::ofstream outputFile((FileSystem::shared().getApplicationPath() + "config.txt").c_str());
        if(!outputFile.is_open()) 
        {
            LOG_WARNING("Could not open output config file!");
            return;
        }
        std::stringstream fileStream(configFile->getData());
        string line;
        while(true)
        {
            getline(fileStream,line);
            if(!fileStream.good()) break;
            if(line.empty())
            {
                outputFile << line << std::endl;
                continue;
            }
            else
            {
                string lineVarAndValue = line.substr(4, line.size()-4);
                string lineVar = line.substr(4, lineVarAndValue.find_first_of(" "));
                if(lineVar == name)
                {
                    if(type == TYPE_STRING)
                    {
                        outputFile << "set " << name << " " << value << " " << "string" << std::endl;
                    }
                    else if(type == TYPE_BOOL)
                    {
                        outputFile << "set " << name << " " << value << " " << "bool" << std::endl;
                    }
                    else if(type == TYPE_FLOAT)
                    {
                        outputFile << "set " << name << " " << value << " " << "float" << std::endl;
                    }
                    else if(type == TYPE_INTEGER)
                    {
                        outputFile << "set " << name << " " << value << " " << "integer" << std::endl;
                    }
                    flag = true;
                }
                else
                {
                    outputFile << line << std::endl;
                }
            }
        }
        if(flag == false)
        {
            if(type == TYPE_STRING)
            {
                outputFile << "set " << name << " " << value << " " << "string" << std::endl;
            }
            else if(type == TYPE_BOOL)
            {
                outputFile << "set " << name << " " << value << " " << "bool" << std::endl;
            }
            else if(type == TYPE_FLOAT)
            {
                outputFile << "set " << name << " " << value << " " << "float" << std::endl;
            }
            else if(type == TYPE_INTEGER)
            {
                outputFile << "set " << name << " " << value << " " << "integer" << std::endl;
            }
        }
        outputFile.close();
        updateConfigFile();
    }
    void Config::setCvarInt(string name, int value)
    {
        std::ostringstream stream;
        stream << value;
        setCvar(name, stream.str());
    }
    void Config::setCvarFloat(string name, float value)
    {
        std::ostringstream stream;
        stream << value;
        setCvar(name, stream.str());
    }
    void Config::setCvarBool(string name, bool value)
    {
        std::ostringstream stream;
        stream << value;
        setCvar(name, stream.str());
    }
    void Config::setConfigFile(File* file)
    {
        configFile = file;
    }

    bool Config::loadConfigFile(string configFileName)
    {
        bool ret = false;
        configFile = FileSystem::shared().getFile(configFileName);
        if(configFile != 0)
        {
            ret = true;
            std::stringstream fileStream(configFile->getData());
            string line;
            while(true)
            {
                getline(fileStream,line);
                if(!fileStream.good()) break;
                if(line.empty()) continue;
                if(line[0] == '#') continue;
                CommandHandler::shared().onCommand(line);
            }
        }
        setConfigFile(configFile);
        return ret;
    }
	bool Config::handleCommand(string command)
	{
		bool flag = true;
		if(CommandHandler::shared().splitLineCommand(command) == "get")
		{
			string variableName;
			string type;
			std::stringstream parser;
			parser << CommandHandler::shared().splitLineParameters(command);
			parser >> variableName >> type;
			if(type == "int" || type == "Int" || type == "integer" || type == "Integer" || type == "INT")
			{
				int output = getCvarInt(variableName);
				LOG_INFO("The value of " << variableName << " is: " << output);
			}
			else if(type == "float" || type == "FLOAT" || type == "Float")
			{
				float output = getCvarFloat(variableName);
				LOG_INFO("The value of " << variableName << " is: " << output);
			}
			else if(type == "bool" || type == "BOOL" || type == "Bool")
			{
				bool output = getCvarBool(variableName);
				LOG_INFO("The value of " << variableName << " is: " << output);
			}
			else if(type == "string" || type == "STRING" || type == "String")
			{
				string output = getCvarString(variableName);
				LOG_INFO("The value of " << variableName << " is: " << output);
			}
			else
			{
				LOG_WARNING("Type of " << variableName << " not recognised!");
			}
		}
		if(CommandHandler::shared().splitLineCommand(command) == "set")
		{
			string variableName;
			string value;
			string type;
			std::stringstream parser;
			parser << CommandHandler::shared().splitLineParameters(command);
			parser >> variableName >> value >> type;
			if(type == "int" || type == "Int" || type == "integer" || type == "Integer" || type == "INT")
			{
				setCvar(variableName, value, TYPE_INTEGER);
			}
			else if(type == "float" || type == "FLOAT" || type == "Float")
			{
				setCvar(variableName, value, TYPE_FLOAT);
			}
			else if(type == "bool" || type == "BOOL" || type == "Bool")
			{
				setCvar(variableName, value, TYPE_BOOL);
			}
			else if(type == "string" || type == "STRING" || type == "String")
			{
				setCvar(variableName, value, TYPE_STRING);
			}
			else LOG_WARNING("Type of " << variableName << " not recognised!");
		}
		else flag = false;
		return flag;
	}

	SettingsManager::SettingsManager()
	{
		Config::shared().setSettingsManager(this);
	}

	SettingsManager::~SettingsManager()
	{

	}

	bool SettingsManager::init()
	{
		vec2 windowSize = vec2(300.0f, 200.0f);
		settingsMenuWindow = new Arya::Window(vec2(0.0f, 0.0f), windowSize * -0.5f, windowSize, 
				TextureManager::shared().getTexture("white"), Arya::WINDOW_DRAGGABLE, "Settings",
				vec4(0.1f, 0.1f, 0.1f, 1.0f));
		controlsMenuWindow = new Arya::Window(vec2(0.0f, 0.0f), windowSize * -0.5f, windowSize, 
				TextureManager::shared().getTexture("white"), Arya::WINDOW_DRAGGABLE, "Controls",
				vec4(0.1f, 0.1f, 0.1f, 1.0f));
		graphicsMenuWindow = new Arya::Window(vec2(0.0f, 0.0f), windowSize * -0.5f, windowSize, 
				TextureManager::shared().getTexture("white"), Arya::WINDOW_DRAGGABLE, "Graphics",
				vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Arya::Button* controlsButton;
		Arya::Button* graphicsButton;
		Arya::Button* closeButton;

		Arya::Button* fullscreenButton;
		Arya::Button* serveraddressButton;
		Arya::Button* goingforwardgameButton;
		Arya::Button* goingbackwardgameButton;
		Arya::Button* rotatingleftgameButton;
		Arya::Button* rotatingrightgameButton;
		Arya::Button* goingleftgameButton;
		Arya::Button* goingrightgameButton;
		Arya::Button* goingdowngameButton;
		Arya::Button* goingupgameButton;
		Arya::Button* synchronizegameButton;
		Arya::Button* backcontrolsButton;

		Arya::Font* f = Arya::FontManager::shared().getFont("DejaVuSans-Bold.ttf");
		Texture* t = TextureManager::shared().getTexture("white");

		controlsButton = new Arya::Button(
				vec2(-1.0, 1.0), vec2(10.0f, -60.0f), vec2(280.0f, 30.0f),
				t, f, "Controls", "controlsButton",
				this, false, vec4(0.5f, 0.5f, 0.5f, 1.0f)
				);
		graphicsButton = new Arya::Button(
				vec2(-1.0, 1.0), vec2(10.0f, -100.0f), vec2(280.0f, 30.0f),
				t, f, "Graphics", "graphicsButton",
				this, false, vec4(0.5f, 0.5f, 0.5f, 1.0f)
				);
		closeButton = new Arya::Button(
				vec2(-1.0, 1.0), vec2(10.0f, -140.0f), vec2(280.0f, 30.0f),
				t, f, "Close Settings", "closeButton",
				this, false, vec4(0.5f, 0.5f, 0.5f, 1.0f)
				);
		settingsMenuWindow->addChild(controlsButton);
		settingsMenuWindow->addChild(graphicsButton);
		settingsMenuWindow->addChild(closeButton);

		Arya::Button* controlButtons[12];
		controlButtons[0] = fullscreenButton;
		controlButtons[1] = serveraddressButton;
		controlButtons[2] = goingforwardgameButton;
		controlButtons[3] = goingbackwardgameButton;
		controlButtons[4] = rotatingleftgameButton;
		controlButtons[5] = rotatingrightgameButton;
		controlButtons[6] = goingleftgameButton;
		controlButtons[7] = goingrightgameButton;
		controlButtons[8] = goingdowngameButton;
		controlButtons[9] = goingupgameButton;
		controlButtons[10] = synchronizegameButton;
		controlButtons[11] = backcontrolsButton;
		string labelControlButtons[12] = {"Fullscreen","Server address","Up","Down","Rotate left","Rotate right","Left",
			"Right","Zoom in","Zoom out","Synchronize server", "Back"};
		string nameControlButtons[12] = {""};
		nameControlButtons[0] = "fullscreenButton";
		nameControlButtons[1] = "serveraddressButton";
		nameControlButtons[2] = "goingforwardgameButton";
		nameControlButtons[3] = "goingbackwardgameButton";
		nameControlButtons[4] = "rotatingleftgameButton";
		nameControlButtons[5] = "rotatingrightgameButton";
		nameControlButtons[6] = "goingleftgameButton";
		nameControlButtons[7] = "goingrightgameButton";
		nameControlButtons[8] = "goingdowngameButton";
		nameControlButtons[9] = "goingupgameButton";
		nameControlButtons[10] = "synchronizegameButton";
		nameControlButtons[11] = "backcontrolsButton";
		for(int i = 0; i < 11; i++)
		{
			labelControlButtons[i].append(" = ").append(Config::shared().getCvar(string(nameControlButtons[i]).substr(0,nameControlButtons[i].find_first_of('B')))->value);
		}
		for(int i = 0; i < 12; i++)
		{
			if(i < 11) controlButtons[i] = new Arya::Button(
					vec2(-1.0, 1.0), vec2(10.0f, -60.0f - 40.0f * i), vec2(280.0f, 30.0f),
					t, f, labelControlButtons[i], nameControlButtons[i],
					this, false, vec4(0.5f, 0.5f, 0.5f, 1.0f)
					);
			else controlButtons[i] = new Arya::Button(
					vec2(-1.0, 1.0), vec2(10.0f, -80.0f - 40.0f * i), vec2(280.0f, 30.0f),
					t, f, labelControlButtons[i], nameControlButtons[i],
					this, false, vec4(0.5f, 0.5f, 0.5f, 1.0f)
					);
			controlsMenuWindow->addChild(controlButtons[i]);
		}
		return true;
	}

	void SettingsManager::cleanup()
	{

	}

	void SettingsManager::changeSetting(string setting, string value)
	{
		Config::shared().setCvar(setting, value, Config::shared().getCvar(setting)->type);
	}

	void SettingsManager::makeMenuActive(Window* window)
	{
		Arya::Interface::shared().makeActive(window);
	}
	
	void SettingsManager::makeMenuInactive(Window* window)
	{
		Arya::Interface::shared().makeInactive(window);
	}

	void SettingsManager::buttonClicked(Arya::Button* sender)
	{
		if(settingsMenuWindow->getActiveState() && sender->getIdentifier() == "controlsButton") 
		{
			makeMenuInactive(settingsMenuWindow);
			makeMenuActive(controlsMenuWindow);
		}
		else if(settingsMenuWindow->getActiveState() && sender->getIdentifier() == "graphicsButton")
		{
			makeMenuInactive(settingsMenuWindow);
			makeMenuActive(graphicsMenuWindow);
		}
		else if(settingsMenuWindow->getActiveState() && sender->getIdentifier() == "closeButton") makeMenuInactive(settingsMenuWindow);

		// controls buttons
		else if(sender->getIdentifier() == "fullscreenButton");
		else if(sender->getIdentifier() == "serveraddressButton");
		else if(sender->getIdentifier() == "goingforwardgameButton");
		else if(sender->getIdentifier() == "goingbackwardgameButton");
		else if(sender->getIdentifier() == "rotatingleftgameButton");
		else if(sender->getIdentifier() == "rotatingrightgameButton");
		else if(sender->getIdentifier() == "goingleftgameButton");
		else if(sender->getIdentifier() == "goingrightgameButton");
		else if(sender->getIdentifier() == "goingdowngameButton");
		else if(sender->getIdentifier() == "goingupgameButton");
		else if(sender->getIdentifier() == "synchronizegameButton");
		else if(sender->getIdentifier() == "backcontrolsButton")
		{
			makeMenuInactive(controlsMenuWindow);
			makeMenuActive(settingsMenuWindow);
		}
	}
}
