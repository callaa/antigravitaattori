#ifndef _M3DMESH_H_
#define _M3DMESH_H_

struct Vertex
{
	float co[3];
	float no[3];
};

struct Face
{
	int verts[3];
	float no[3];
	float uv[3][2];
	int texture;
	int material;
	int smooth;
};

struct FaceSort
{
	int operator()(const struct Face &face1, const struct Face &face2)
	{
		if(face1.texture < face2.texture) return 1;
		else if(face1.texture > face2.texture) return 0;
		else return face1.material < face2.material;
	}
};

class m3dMaterial;

/// A triangle mesh
/**
	The m3dMesh is a simple class for loading and drawing triangle mesh
	models.
*/
class m3dMesh
{
public:
	m3dMesh();
	~m3dMesh();

	int loadFromXML(const TiXmlElement *root);
	int loadFromXML(const char *filename);
	
	const m3dTexture &getTexture(int n) const;
	void setTexture(int n, const m3dTexture &tex);
	
	void draw();
	
private:
	struct Vertex *verts;
	struct Face *faces;
	int numVerts;
	int numFaces;
	
	m3dTexture *textures;
	int numTextures;
	
	m3dMaterial *materials;
	int numMaterials;
	
	int parseVertex(const TiXmlElement *root, struct Vertex *vert);
	int parseFace(const TiXmlElement *root, struct Face *face);
	
	void writeVertex(TiXmlElement *root, const struct Vertex *vert);
	void writeFace(TiXmlElement *root, const struct Face *face);

};

#endif

