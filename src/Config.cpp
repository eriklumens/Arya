#include <sstream>
#include "common/Logger.h"
#include "Config.h"
#include "Console.h"
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
        cleanup();
    }

    bool Config::init()
    {
        setCvarWithoutSave("fullScreen", "true", TYPE_BOOL);
        return loadConfigFile("config.txt");
    }

    void Config::cleanup()
    {
        if(configFile != 0) FileSystem::shared().releaseFile(configFile);
        configFile = 0;
    }

    bool Config::loadConfigFile(string configFileName)
    {
        configFile = FileSystem::shared().getFile(configFileName);
        if(configFile == 0) return false;
        std::stringstream fileStream(configFile->getData());
        string regel;
        while(true)
        {
            getline(fileStream,regel);
            if(!fileStream.good()) break;
            if(regel.empty()) continue;
            if(regel.substr(0,3) == "set") continue;
            if(regel[0] == '#') continue;
            Console::shared().onCommand(regel);
        }
        return true;
    }
    bool Config::loadConfigFileAfterRootInit(string configFileName)
    {
        configFile = FileSystem::shared().getFile(configFileName);
        if(configFile == 0) return false;
        std::stringstream fileStream(configFile->getData());
        string regel;
        while(true)
        {
            getline(fileStream,regel);
            if(!fileStream.good()) break;
            if(regel.empty()) continue;
            if(regel.substr(0,3) == "set") continue;
            if(regel[0] == '#') continue;
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
        bool flag = false;
        std::ofstream outputFile((FileSystem::shared().getApplicationPath() + "config.txt").c_str());
        if(!outputFile.is_open()) 
        {
            LOG_WARNING("Could not open output config file!");
            return;
        }
        string editCommand = edit.substr(0, edit.find_first_of(" "));
        std::stringstream fileStream(configFile->getData());
        string regel;
        while(true)
        {
            getline(fileStream,regel);
            if(!fileStream.good()) break;
            if(regel.empty())
            {
                outputFile << regel << std::endl;
                continue;
            }
            else
            {
                string regelCommand = regel.substr(0, edit.find_first_of(" "));
                if(regelCommand == editCommand)
                {
                    outputFile << edit << std::endl;
                    flag = true;
                }
                else
                {
                    outputFile << regel << std::endl;
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
        LOG_INFO("I do not get stuck here!");
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
    bool Config::getCvarBool(string name)
    {
        cvar* var= getCvar(name);
        if(var) return var->getBool();
        else
        {
            LOG_WARNING("Var " << name << " is not of this type!");
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
            var = getCvar(name);
        }
        if(var == 0)
        {
            LOG_ERROR("Could not add new cvar '" << name << "' to list");
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
        bool flag = false;
        std::ofstream outputFile((FileSystem::shared().getApplicationPath() + "config.txt").c_str());
        if(!outputFile.is_open()) 
        {
            LOG_WARNING("Could not open output config file!");
            return;
        }
        std::stringstream fileStream(configFile->getData());
        string regel;
        while(true)
        {
            getline(fileStream,regel);
            if(!fileStream.good()) break;
            if(regel.empty())
            {
                outputFile << regel << std::endl;
                continue;
            }
            else
            {
                string regelVarAndValue = regel.substr(4, regel.size()-4);
                string regelVar = regel.substr(4, regelVarAndValue.find_first_of(" "));
                if(regelVar == name)
                {
                    outputFile << "set " << name << " " << value << std::endl;
                    flag = true;
                }
                else
                {
                    outputFile << regel << std::endl;
                }
            }
        }
        if(flag == false) outputFile << "set " << name << " " << value << std::endl;
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
}