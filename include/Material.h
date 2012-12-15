#pragma once
#include "Textures.h"
#include "common/Singleton.h"
#include <vector>
#include <map>

namespace Arya
{
    class Material
    {
        public:
			Material(std::string _name, Texture* _texture, std::string _type, float _specAmp, float _specPow, float _ambient, float _diffuse){
				name=_name; texture=_texture; type=_type; specAmp=_specAmp; specPow=_specPow; ambient=_ambient; diffuse=_diffuse;
			}
			~Material(){}
            
			std::string name;
			Texture* texture;
			std::string type;
			float specAmp;	// The "amount" of highlights
			float specPow;	// The "sharpness" of highlights
			float ambient;  // The "amount" of ambient lighting
			float diffuse;  // The "amount" of diffuse lighting
		private:
			
    };

	class MaterialManager : public Singleton<MaterialManager>, public ResourceManager<Material> {
        public:
			MaterialManager(){}
			~MaterialManager(){cleanup();}

            bool initialize(std::vector<const char*> filenames);
            void cleanup();

			Material* getMaterial( const char* filename ){ return getResource(filename); }

        private:
			Material* loadResource(const char* filename);
    };
};
