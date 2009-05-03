#include "level.h"

#include "gfx.h"

#include "material.h"

#include "../util/vec.h"
#include "../util/math_func.h"


#include <allegro.h>
#include <string>
#include <vector>

using namespace std;

Level::Level()
{
	loaded = false;

	material = NULL;

	// Rock
	m_materialList[0].worm_pass = false;
	m_materialList[0].particle_pass = false;
	m_materialList[0].draw_exps = false;
	m_materialList[0].blocks_light = true;
	
	// Background
	m_materialList[1].worm_pass = true;
	m_materialList[1].particle_pass = true;
	m_materialList[1].draw_exps = true; // I added this coz its cute :P
	
	// Dirt
	m_materialList[2].worm_pass = false;
	m_materialList[2].particle_pass = false;
	m_materialList[2].draw_exps = false;
	m_materialList[2].destroyable = true;
	m_materialList[2].blocks_light = true;
	
	// Special dirt
	m_materialList[3].worm_pass = true;
	m_materialList[3].particle_pass = false;
	m_materialList[3].draw_exps = false;
	m_materialList[3].destroyable = true;
	
	// Special rock
	m_materialList[4].worm_pass = false;
	m_materialList[4].particle_pass = true;
	m_materialList[4].draw_exps = false;
	
	m_materialList[7].worm_pass = false;
	m_materialList[7].particle_pass = false;
	m_materialList[7].draw_exps = false;
}

Level::~Level()
{
}

void Level::unload()
{
	loaded = false;
	path = "";

	destroy_bitmap(material); material = NULL;

}

bool Level::isLoaded()
{
	return loaded;
}

Vec Level::getSpawnLocation()
{
	Vec pos = Vec(rnd() * material->w, rnd()*material->h);
	while ( !getMaterial( static_cast<int>(pos.x), static_cast<int>(pos.y) ).worm_pass )
	{
		pos = Vec(rnd() * material->w, rnd()*material->h);
	}
	return pos;
}

/*
const Material& Level::getMaterial(int x, int y)
{
	return m_materialList[getpixel(material,x,y)+1];
}
*/

int Level::width()
{
	if ( material )
		return material->w;
	else
		return 0;
}

int Level::height()
{
	if ( material )
		return material->h;
	else
		return 0;
}

void Level::setName(const std::string& _name)
{
	name = _name;
}

const string& Level::getPath()
{
	return path;
}

const string& Level::getName()
{
	return name;
}

void Level::loaderSucceeded()
{
	loaded = true;
}
