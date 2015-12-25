#define GL_GLEXT_PROTOTYPES
#include "SDL_opengl.h"
#include <stdio.h>
#include <string>
#include <algorithm>

using namespace std;

#include "tinyxml.h"
#include "m3dmaterial.h"
#include "m3dtexture.h"
#include "m3dmesh.h"

#include "extensions.h"

/// Create a new empty mesh
/**
*/
m3dMesh::m3dMesh()
{
	verts = NULL;
	numVerts = 0;
	faces = NULL;
	numFaces = 0;
	materials = NULL;
	numMaterials = 0;
	textures = NULL;
	numTextures = 0;
}

/// Destroy this mesh
/**
*/
m3dMesh::~m3dMesh()
{
	delete[] verts;
	delete[] faces;
	delete[] materials;
	delete[] textures;
}

int m3dMesh::loadFromXML(const TiXmlElement *root)
{
	if(string(root->Value()) != "Mesh")
	{
		fprintf(stderr, "Unknown node type: %s  (required: %s)\n", root->Value(), "Mesh");
		return -1;
	}

// 	if(readTransformations(root) != 0) return -1;

	if(root->QueryIntAttribute("numVertices", &numVerts) != TIXML_SUCCESS) return -1;
	if(root->QueryIntAttribute("numFaces", &numFaces) != TIXML_SUCCESS) return -1;

	faces = new struct Face[numFaces];
	verts = new struct Vertex[numVerts];
	int f = 0, v = 0;

	numMaterials = 0;

	const TiXmlElement *element = root->FirstChildElement();
	string value;
	while(element)
	{
		value = element->Value();

		if(value == "Vertex")
		{
			if(parseVertex(element, &verts[v]) != 0 || v >= numVerts)
			{
				fprintf(stderr, "Invalid vertex!\n");
				delete[] verts;
				delete[] faces;
				return -1;
			}

			v++;
		} else if(value == "Face")
		{
			if(parseFace(element, &faces[f]) != 0 || f >= numFaces)
			{
				fprintf(stderr, "Invalid face!\n");
				delete[] verts;
				delete[] faces;
				return -1;
			}

			if(faces[f].material > numMaterials-1) numMaterials = faces[f].material+1;
			if(faces[f].texture > numTextures-1) numTextures = faces[f].texture+1;
			f++;
		}

		element = element->NextSiblingElement();
	}

	materials = new m3dMaterial[numMaterials];
	int mat = 0;
	element = root->FirstChildElement("Material");
	while(element)
	{
		if(mat >= numMaterials)
		{
			fprintf(stderr, "Invalid mesh: incorrect number of materials!\n");
			return -1;
		}

		materials[mat].loadFromXML(element);
		mat++;

		element = element->NextSiblingElement("Material");
	}

	if(mat != numMaterials)
	{
		fprintf(stderr, "Invalid mesh: incorrect number of materials (wanted %d, got %d)!\n", numMaterials, mat);
		return -1;
	}

	textures = new m3dTexture[numTextures];
	mat = 0;

	element = root->FirstChildElement("Texture");
	while(element)
	{
		if(mat >= numTextures)
		{
			fprintf(stderr, "Invalid mesh: incorrect number of textures\n");
			return -1;
		}

		textures[mat].loadFromXML(element);
		mat++;

		element = element->NextSiblingElement("Texture");
	}


	if(mat != numTextures)
	{
		fprintf(stderr, "Invalid mesh: incorrect number of textures (wanted %d, got %d)!\n", numTextures, mat);
		return -1;
	}

	std::sort(faces, faces+numFaces, FaceSort());

	return 0;
}

int m3dMesh::parseVertex(const TiXmlElement *root, struct Vertex *vert)
{
	if(string(root->Value()) != "Vertex")
	{
		fprintf(stderr, "Unknown node type: %s  (required: Vertex)\n", root->Value());
		return -1;
	}

	if(root->QueryFloatAttribute("x", &vert->co[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("y", &vert->co[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("z", &vert->co[2]) != TIXML_SUCCESS) return -1;

	if(root->QueryFloatAttribute("nx", &vert->no[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("ny", &vert->no[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("nz", &vert->no[2]) != TIXML_SUCCESS) return -1;

	return 0;
}

int m3dMesh::parseFace(const TiXmlElement *root, struct Face *face)
{
	if(string(root->Value()) != "Face")
	{
		fprintf(stderr, "Unknown node type: %s  (required: Face)\n", root->Value());
		return -1;
	}

	if(root->QueryIntAttribute("smooth", &face->smooth) != TIXML_SUCCESS) return -1;

	if(root->QueryFloatAttribute("nx", &face->no[0]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("ny", &face->no[1]) != TIXML_SUCCESS) return -1;
	if(root->QueryFloatAttribute("nz", &face->no[2]) != TIXML_SUCCESS) return -1;

	if(root->QueryIntAttribute("material", &face->material) != TIXML_SUCCESS) return -1;
	if(root->QueryIntAttribute("texture", &face->texture) != TIXML_SUCCESS) return -1;

	const TiXmlElement *element = root->FirstChildElement("Vertex");
	for(int i = 0; i < 3; i++)
	{
		if(element == NULL)
		{
			return -1;
		}

		if(element->QueryIntAttribute("index", &face->verts[i]) != TIXML_SUCCESS) return -1;
		if(element->QueryFloatAttribute("u", &face->uv[i][0]) == TIXML_WRONG_TYPE) return -1;
		if(element->QueryFloatAttribute("v", &face->uv[i][1]) == TIXML_WRONG_TYPE) return -1;

		element = element->NextSiblingElement("Vertex");
	}

	return 0;
}

int m3dMesh::loadFromXML(const char *filename)
{
	TiXmlDocument doc;
	if(!doc.LoadFile(filename)) return -1;
	return loadFromXML(doc.RootElement());
}

const m3dTexture &m3dMesh::getTexture(int n) const
{
	return textures[n];
}

void m3dMesh::setTexture(int n, const m3dTexture& tex)
{
	textures[n] = tex;
}

/// Draw the mesh
/**
	Draws this mesh. No child objects are rendered, nor child lights are enabled.
*/
void m3dMesh::draw()
{
	glPushMatrix();
// 	transform();

	int prevTexture;
	int prevMaterial;
#ifdef HAVE_MULTITEX
	int numTexUnits;
#endif

	if(faces[0].material != -1)
	{
		materials[faces[0].material].bind();
	}
	prevMaterial = faces[0].material;

	if(faces[0].texture != -1)
	{
		textures[faces[0].texture].bind();
#ifdef HAVE_MULTITEX
		numTexUnits = textures[faces[0].texture].getNumTexUnits();
#endif
		glEnable(GL_TEXTURE_2D);
	} else
	{
		glDisable(GL_TEXTURE_2D);
#ifdef HAVE_MULTITEX
		numTexUnits = 0;
#endif
	}
	prevTexture = faces[0].texture;

	glBegin(GL_TRIANGLES);
	for(int i = 0; i < numFaces; i++)
	{
		struct Face *face;
		face = &faces[i];

		if(prevMaterial != face->material || prevTexture != face->texture) glEnd();

		if(prevMaterial != face->material)
		{
			if(face->material != -1)
			{
				materials[face->material].bind();
			} else
			{
				m3dMaterial().bind();
			}

			prevMaterial = face->material;

			if(prevTexture == face->texture) glBegin(GL_TRIANGLES);
		}

		if(prevTexture != face->texture)
		{
			if(face->texture != -1)
			{
				textures[face->texture].bind();
#ifdef HAVE_MULTITEX
				numTexUnits = textures[face->texture].getNumTexUnits();
#endif
				if(prevTexture == -1) glEnable(GL_TEXTURE_2D);
			} else
			{
#ifdef HAVE_MULTITEX
				numTexUnits = 0;
#endif
				glDisable(GL_TEXTURE_2D);
			}

			prevTexture = face->texture;

			glBegin(GL_TRIANGLES);
		}

		if(!face->smooth)
		{
			glNormal3fv(face->no);
		}

		for(int j = 0; j < 3; j++)
		{
			struct Vertex *vert;
			vert = &verts[face->verts[j]];

#ifdef HAVE_MULTITEX
			for(int t = 0; t < numTexUnits; t++)
			{
				mglMultiTexCoord2fv(GL_TEXTURE0_ARB + t, face->uv[j]);
			}
#else
            glTexCoord2fv(face->uv[j]);
#endif

			if(face->smooth) glNormal3fv(vert->no);
			glVertex3fv(vert->co);
		}

	}

	glEnd();
	glPopMatrix();
}
