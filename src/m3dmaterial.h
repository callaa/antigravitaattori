#ifndef _M3DMATERIAL_H_
#define _M3DMATERIAL_H_

/// The base for all graphical objects
/**

*/
class m3dMaterial
{
	public:
	m3dMaterial();
	~m3dMaterial();
	
	int loadFromXML(const TiXmlElement *root);
	int saveToXML(TiXmlElement *root);

	void bind();
	
	protected:
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
};

#endif
