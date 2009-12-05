#ifndef DEF_H
#define DEF_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

typedef unsigned char BYTE;

struct TC;

struct WeapSettings
{
	WeapSettings()
	: written(false)
	{
	}

	struct ObjGovernor
	{

	};

	enum
	{
		SHOTTYPE_LASER = 4,
	};

	BYTE detectdistance;
	bool affectbywormspeed;
	char blowaway;
	BYTE order;
	short gravity; // *
	bool shadow;
	bool lasersight;
	BYTE launchsound;
	BYTE loopsound;
	BYTE explosound;
	short speed; // *
	short addspeed; // *
	short distribution;
	BYTE parts; // *
	char recoil;
	short multspeed;
	short delay; // *
	short loadingtime;
	BYTE ammo;
	BYTE createonexp;
	char dirteffect;
	BYTE leaveshells;
	BYTE leaveshelldelay;
	bool playreloadsound;
	bool wormexplode;
	bool explground; // .
	BYTE wormcollide;
	BYTE firecone;
	bool collidewithobjects;
	bool affectbyexplosions;
	BYTE bounce; // *
	short timetoexplosion;  // *
	short timetoexplosionv; // *
	BYTE hitdamage;
	BYTE bloodonhit;
	short startframe;    // .
	BYTE numframes;
	bool loopanim;
	BYTE shottype;       // .
	BYTE colorbullets;   // *
	BYTE splinteramount; // *
	BYTE splintercolor;
	BYTE splintertype;   // *
	BYTE splinterscatter;
	BYTE objtrailtype;
	BYTE objtraildelay;
	BYTE parttrailtype;
	BYTE parttrailobj;   // *
	BYTE parttraildelay; // *

	char name[14];

	std::string writeWeapon();
	std::string writeWObj();
	void writeWObjExplActions(std::ostream& f);

	int idx;
	bool written;
	std::string writtenName;
	bool objWritten;
	std::string objWrittenName;

	TC* tc;
};

struct ObjSettings
{
	ObjSettings()
	: written(false)
	{
	}

	BYTE detectdistance;
	short gravity; // *
	short speed;   // .
	short speedv;  // .
	short distribution;
	char blowaway;
	BYTE bounce; // *
	char hitdamage;
	bool wormexplode;
	bool explground; // .
	bool wormdestroy;
	BYTE bloodonhit;
	BYTE startframe; // .
	BYTE numframes;
	bool drawonmap;
	BYTE colorbullets; // *
	BYTE createonexp;
	bool affectbyexplosions;
	char dirteffect;
	BYTE splinteramount; // *
	BYTE splintercolor;
	BYTE splintertype;   // *
	bool bloodtrail;      // .
	BYTE bloodtraildelay; // .
	BYTE leaveobj;
	BYTE leaveobjdelay;
	short timetoexplosion;  // *
	short timetoexplosionv; // *

	void writeObjExplActions(std::ostream& f);
	std::string writeObj();

	int  idx;
	bool written;
	std::string writtenName;

	TC* tc;
};

struct Sound
{
	Sound()
	: written(false)
	{
	}

	std::vector<char> data;

	std::streamoff offset;
	std::streamoff length;
	char name[9];

	std::string writeSound();

	bool written;
	std::string writtenName;

	TC* tc;
};

struct RGB
{
	int r;
	int g;
	int b;
};

struct TC
{
	TC(fs::path const& dest_)
	: dest(dest_)
	{
		int i;
		for(i = 0; i < 40; ++i)
		{
			w[i].tc = this;
			w[i].idx = i + 1;
		}

		for(i = 0; i < 24; ++i)
		{
			o[i].tc = this;
			o[i].idx = i + 1;
		}
	}

	WeapSettings w[40];
	ObjSettings o[24];
	RGB palette[256];
	std::vector<Sound> s;
	fs::path dest;
	void read(std::istream&, std::istream&);
};

#endif //DEF_H
