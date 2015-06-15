bin_PROGRAMS = antigrav

INCLUDES = -W -Wall -DTIXML_USE_STL -Itinyxml/ -DDATADIR="\"$(datadir)/$(PACKAGE)\""
SUBDIRS = tinyxml
LDADD = tinyxml/libtinyxml.a

antigrav_SOURCES = main.cpp antigrav.h extensions.h \
		craft.cpp craft.h \
		level.cpp level.h \
		vector2.cpp vector2.h \
		font.cpp font.h \
		m3dmaterial.cpp m3dmaterial.h \
		m3dmesh.cpp m3dmesh.h \
		m3dtexture.cpp m3dtexture.h \
		terrain.cpp terrain.h \
		game.cpp game.h \
		player.cpp player.h \
		menu.cpp menu.h \
	 	ring.cpp ring.h \
		background.cpp background.h

