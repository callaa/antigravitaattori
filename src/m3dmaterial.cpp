#include "SDL_opengl.h"
#include <string>

using namespace std;

#include "tinyxml.h"
#include "m3dmaterial.h"
#include "m3dtexture.h"
#include "m3dmesh.h"

/// Create a new material
/**
	Create a new material and make it dull gray
*/
m3dMaterial::m3dMaterial()
{
	ambient[0] = 0.2;
	ambient[1] = 0.2;
	ambient[2] = 0.2;
	
	diffuse[0] = 0.7;
	diffuse[1] = 0.7;
	diffuse[2] = 0.7;
	
	specular[0] = 1.0;
	specular[1] = 1.0;
	specular[2] = 1.0;
	
	shininess = 0.5;
}

m3dMaterial::~m3dMaterial()
{
}

/// Load an object from XML
/**
	@param root the XML element that represents this object
	@return -1 on failure, 0 on success
*/
int m3dMaterial::loadFromXML(const TiXmlElement *root)
{
	if(root->QueryFloatAttribute("ambientR", &ambient[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("ambientG", &ambient[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("ambientB", &ambient[2]) != TIXML_SUCCESS) return -1;
	
	if(root->QueryFloatAttribute("diffuseR", &diffuse[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("diffuseG", &diffuse[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("diffuseB", &diffuse[2]) != TIXML_SUCCESS) return -1;
	
	if(root->QueryFloatAttribute("specularR", &specular[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("specularG", &specular[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("specularB", &specular[2]) != TIXML_SUCCESS) return -1;
	
	if(root->QueryFloatAttribute("shininess", &shininess) != TIXML_SUCCESS) return -1;
	
	return 0;
}

void m3dMaterial::bind()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess*128.0f);
}

