#!BPY
"""
Name: 'm3d mesh exporter'
Blender: 237
Group: 'Export'
Tooltip: 'Export meshes to .xml'
"""

import Blender
from Blender import NMesh, Object, Scene, Lamp

import math
from math import sin, cos

import os

global indentation
indentation = 0

def indent(file):
	if indentation < 1:
		return
	
	for i in range(indentation):
		file.write("\t");
		
def setIndent(d):
	global indentation
	indentation += d

def toQuaternion(fRoll, fYaw, fPitch):
	fSinPitch = sin(fPitch*0.5)
	fCosPitch = cos(fPitch*0.5)
	fSinYaw = sin(fYaw*0.5)
	fCosYaw = cos(fYaw*0.5)
	fSinRoll = sin(fRoll*0.5)
	fCosRoll = cos(fRoll*0.5)
	fCosPitchCosYaw = fCosPitch*fCosYaw
	fSinPitchSinYaw = fSinPitch*fSinYaw
	X = fSinRoll * fCosPitchCosYaw     - fCosRoll * fSinPitchSinYaw
	Y = fCosRoll * fSinPitch * fCosYaw + fSinRoll * fCosPitch * fSinYaw
	Z = fCosRoll * fCosPitch * fSinYaw - fSinRoll * fSinPitch * fCosYaw
	W = fCosRoll * fCosPitchCosYaw     + fSinRoll * fSinPitchSinYaw
	return [W, X, Y, Z]

def export(obj, file):
	print "Export %s\n" %obj.getName()
	
	locX = obj.LocX
	locY = obj.LocY
	locZ = obj.LocZ
	rotX = obj.RotX
	rotY = obj.RotY
	rotZ = obj.RotZ
	sizeX = obj.SizeX
	sizeY = obj.SizeY
	sizeZ = obj.SizeZ
	
	parent = obj.getParent()
	while parent:
		locX -= parent.LocX
		locY -= parent.LocY
		locZ -= parent.LocZ
		rotX -= parent.RotX
		rotY -= parent.RotY
		rotZ -= parent.RotZ
		sizeX /= parent.SizeX
		sizeY /= parent.SizeY
		sizeZ /= parent.SizeZ
		parent = parent.getParent()
		
	loc = [locX, locY, locZ]
	size = [sizeX, sizeY, sizeZ]
	quat = toQuaternion(rotX, -rotY, rotZ)
	
	if obj.getType() == "Mesh":
		exportMesh(obj, file, loc, size, quat)
	
def exportMaterial(mat, file):
	indent(file)
	file.write("<Material name=\"%s\" " % mat.getName())
	col = mat.getRGBCol()
	file.write("diffuseR=\"%1.4f\" diffuseG=\"%1.4f\" diffuseB=\"%1.4f\" " % (col[0], col[1], col[2]))
	amb = mat.getAmb()
	col[0] *= amb
	col[1] *= amb
	col[2] *= amb
	file.write("ambientR=\"%1.4f\" ambientG=\"%1.4f\" ambientB=\"%1.4f\" " % (col[0], col[1], col[2]))
	col = mat.getSpecCol()
	file.write("specularR=\"%1.4f\" specularG=\"%1.4f\" specularB=\"%1.4f\" " % (col[0], col[1], col[2]))
	file.write("shininess=\"%1.4f\"" % (mat.getSpec()))
	file.write("/>\n")
	
def exportTexture(tex, file):
	indent(file)
	file.write("<Texture name=\"%s\" units=\"1\">\n" % tex.getName())
	indent(file)
	file.write("\t<Image filename=\"%s\"/>\n" % os.path.basename(tex.getFilename()))
	indent(file)
	file.write("</Texture>\n")

def exportMesh(obj, file, loc, size, quat):
	mesh = NMesh.GetRawFromObject(obj.getName())
	
	indent(file)
	file.write("<Mesh name=\"%s\" numVertices=\"%d\" numFaces=\"%d\" " %(obj.getName(), len(mesh.verts), len(mesh.faces)))
	file.write("x=\"%3.5f\" y=\"%3.5f\" z=\"%3.5f\" " %(loc[0], loc[2], -loc[1]))
	file.write("sx=\"%3.5f\" sy=\"%3.5f\" sz=\"%3.5f\" " %(size[0], size[1], size[2]))
	file.write("qw=\"%3.5f\" qx=\"%3.5f\" qy=\"%3.5f\" qz=\"%3.5f\"" %(quat[0], quat[1], quat[2], quat[3]))
	file.write(">\n")
	setIndent(1)
	
	for mat in mesh.materials:
		exportMaterial(mat, file)
	if len(mesh.materials) > 0:
		file.write("\n")
		
	textures = []
	if mesh.hasFaceUV():
		for face in mesh.faces:
			if not face.image:
				continue;
				
			if textures.count(face.image) == 0:
				textures.append(face.image)
				
	for tex in textures:
		exportTexture(tex, file)
	if len(textures) > 0:
		file.write("\n")

	for vert in mesh.verts:
		indent(file)
		file.write("<Vertex x=\"%3.5f\" y=\"%3.5f\" z=\"%3.5f\" " %(vert.co[0], vert.co[2], -vert.co[1]))
		file.write("nx=\"%3.5f\" ny=\"%3.5f\" nz=\"%3.5f\"/>\n" %(vert.no[0], vert.no[2], -vert.no[1]))
	
	file.write("\n")
	
	for face in mesh.faces:
		indent(file)
		file.write("<Face smooth=\"%d\" " %face.smooth)
		file.write("nx=\"%3.5f\" ny=\"%3.5f\" nz=\"%3.5f\" " %(face.normal[0], face.normal[2], -face.normal[1]))
		
		if face.image:
			file.write("texture=\"%d\" " %(textures.index(face.image)))
		else:
			file.write("texture=\"-1\" ")
		
		mat = face.materialIndex
		if mat >= len(mesh.materials):
			mat = -1
		
		file.write("material=\"%d\">\n" %(mat))
		
		setIndent(1)
		for i in range(3):
			indent(file);
			file.write("<Vertex index=\"%d\" " %face.v[i].index )
			
			if mesh.hasFaceUV() and face.image:
				file.write("u=\"%1.3f\" v=\"%1.3f\"" %(face.uv[i][0], 1.0-face.uv[i][1]))
			
			file.write("/>\n")

		setIndent(-1)
		
#		file.write(" vertex1=\"%d\" vertex2=\"%d\" vertex3=\"%d\"/>\n" %(face.v[0].index, face.v[1].index, face.v[2].index))
		indent(file)
		file.write("</Face>\n")
	
	Object.GetSelected().remove(obj)
	for child in Object.GetSelected():
		if child.parent == obj:
			export(child, file)
			Object.GetSelected().remove(child)
	
	setIndent(-1)
	indent(file)
	file.write("</Mesh>\n\n")

def main():
	selected = Object.GetSelected()
	
	if len(selected) == 0:
		print "Nothing selected"
		return
	
	for obj in selected:
		if obj.getType() != "Mesh":
			continue
		
		file = open("%s.xml" % (obj.getName()), "w")
		file.write("<? xml version=\"1.0\" ?>\n")
		export(obj, file)
		file.close()
	
	
main()