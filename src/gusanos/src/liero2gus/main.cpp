#include "def.h"

#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
namespace fs = boost::filesystem;

#define READ_W(x_) {for(size_t i = 0; i < sizeof(w)/sizeof(*w); ++i) f.read((char *)&w[i].x_, sizeof(w[i].x_)); }
#define READ_O(x_) {for(size_t i = 0; i < sizeof(o)/sizeof(*o); ++i) f.read((char *)&o[i].x_, sizeof(o[i].x_)); }
#define WRITE(x_) f.write((char *)(x_), sizeof(x_))

double acceleration(short v)
{
	return double(v) * (70.0*70.0/(100.0*100.0*65536.0));
}

double accelerationRatio(short v)
{
	return double(v) * (70.0*70.0/(100.0*100.0*100.0));
}

double velocity(short v)
{
	return double(v) * (70.0/(100.0*65536.0));
}

double velocityFactor(short v)
{
	return double(v) * (70.0/(100.0*100.0));
}

double factor(BYTE v)
{
	return double(v) / 100.0; //TODO: Use (double)(char)v  if the signed hack(s) are activated
}

int time(short v)
{
	return int(double(v) * (100.0/70.0) + 0.5);
}

int ref(BYTE b)
{
	return int(b) - 1;
}

static unsigned char const waveHeader[] = {82,73,70,70,2,236,0,0,87,65,86,69,102,109,116,
32,16,0,0,0,1,0,1,0,34,86,0,0,34,86,0,
0,1,0,8,0,100,97,116,97,};

void createFolders(fs::path const& path)
{
	fs::create_directories(path.branch_path());
}

void TC::read(std::istream& f, std::istream& soundStream)
{
	int i;

	f.seekg(112806);

	cout << "Reading weapons..." << endl;
	READ_W(detectdistance);
	READ_W(affectbywormspeed);
	READ_W(blowaway);
	READ_W(order);
	READ_W(gravity);
	READ_W(shadow);
	READ_W(lasersight);
	READ_W(launchsound);
	READ_W(loopsound);
	READ_W(explosound);
	READ_W(speed);
	READ_W(addspeed);
	READ_W(distribution);
	READ_W(parts);
	READ_W(recoil);
	READ_W(multspeed);
	READ_W(delay);
	READ_W(loadingtime);
	READ_W(ammo);
	READ_W(createonexp);
	READ_W(dirteffect);
	READ_W(leaveshells);
	READ_W(leaveshelldelay);
	READ_W(playreloadsound);
	READ_W(wormexplode);
	READ_W(explground);
	READ_W(wormcollide);
	READ_W(firecone);
	READ_W(collidewithobjects);
	READ_W(affectbyexplosions);
	READ_W(bounce);
	READ_W(timetoexplosion);
	READ_W(timetoexplosionv);
	READ_W(hitdamage);
	READ_W(bloodonhit);
	READ_W(startframe);
	READ_W(numframes);
	READ_W(loopanim);
	READ_W(shottype);
	READ_W(colorbullets);
	READ_W(splinteramount);
	READ_W(splintercolor);
	READ_W(splintertype);
	READ_W(splinterscatter);
	READ_W(objtrailtype);
	READ_W(objtraildelay);
	READ_W(parttrailtype);
	READ_W(parttrailobj);
	READ_W(parttraildelay);

	f.seekg(111430);

	cout << "Reading objects..." << endl;
	READ_O(detectdistance);
	READ_O(gravity);
	READ_O(speed);
	READ_O(speedv);
	READ_O(distribution);
	READ_O(blowaway);
	READ_O(bounce);
	READ_O(hitdamage);
	READ_O(wormexplode);
	READ_O(explground);
	READ_O(wormdestroy);
	READ_O(bloodonhit);
	READ_O(startframe);
	READ_O(numframes);
	READ_O(drawonmap);
	READ_O(colorbullets);
	READ_O(createonexp);
	READ_O(affectbyexplosions);
	READ_O(dirteffect);
	READ_O(splinteramount);
	READ_O(splintercolor);
	READ_O(splintertype);
	READ_O(bloodtrail);
	READ_O(bloodtraildelay);
	READ_O(leaveobj);
	READ_O(leaveobjdelay);
	READ_O(timetoexplosion);
	READ_O(timetoexplosionv);

	// Weapon names
	cout << "Reading weapon names..." << endl;
	f.seekg(0x1B676);
	for(i = 0; i < 40; i++)
	{
		BYTE len;
		f.read((char *)&len, 1);
		f.read(w[i].name, 13);
		w[i].name[len] = 0;
		//cout << "Read " << w[c].name << endl;
	}

	f.seekg(132774);

	cout << "Reading palette..." << endl;
	for(i = 0; i < 256; ++i)
	{
		palette[i].r = f.get() << 2;
		palette[i].g = f.get() << 2;
		palette[i].b = f.get() << 2;
	}

	// Sound names
	soundStream.seekg(0);
	short count = 0;
	soundStream.read((char *)&count, 2);
	cout << "Reading sounds (" << count << " sounds)..." << endl;

	s.assign((size_t)count, Sound());

	i = 1;
	{for(vector<Sound>::iterator siter = s.begin(); siter != s.end(); ++siter, ++i)
	{
		siter->tc = this;
		soundStream.read(siter->name, 8);
		soundStream.read((char *)&siter->offset, 4);
		soundStream.read((char *)&siter->length, 4);
	}}

	cout << "Sound headers read. Reading sound data..." << endl;

	{for(vector<Sound>::iterator siter = s.begin(); siter != s.end(); ++siter)
	{
		soundStream.seekg(siter->offset);
		siter->data.resize(siter->length);
		soundStream.read(&siter->data[0], siter->length);
		for(vector<char>::iterator dataiter = siter->data.begin(); dataiter != siter->data.end(); ++dataiter)
		{
			*dataiter += -128;
		}
	}}
	
	cout << "Liero exe and Liero sound file read successfully." << endl;
}

void filterName(std::string& name)
{
	for(string::iterator i = name.begin(); i != name.end(); ++i)
	{
		char c = tolower(*i);
		if(!isalnum(c) && c != '.' && c != '_' && c != '-')
			c = '-';
		*i = c;
	}
}

string Sound::writeSound()
{
	if(written)
		return writtenName;

	writtenName = string(name) + ".wav";
	filterName(writtenName);
	written = true;

	fs::path p(tc->dest / "sounds" / writtenName);
	createFolders(p);
	fs::ofstream f(p, ios::binary);

	f.write((char const*)waveHeader, sizeof(waveHeader));
	long length = data.size();
	f.write((char *)&length, 4);
	f.write(&data[0], data.size());

	return writtenName;
}

void WeapSettings::writeWObjExplActions(std::ostream& f)
{
	if(splinteramount > 0 && splintertype != 0)
		f << "  shoot_particles(" << tc->o[ref(splintertype)].writeObj() << ", " << (int)splinteramount << ", "
		  << velocityFactor(tc->o[ref(splintertype)].speed) << ", " << velocityFactor(tc->o[ref(splintertype)].speedv) <<
		  ", <motion inheritance>, 0, <angle variation>, 0)" << endl;
	if(explosound > 0)
		f << "  play_sound_static(" << tc->s[ref(explosound)].writeSound() << ")" << endl;
	f << "  remove()" << endl;
	//TODO: createonexp and dirteffect
	
}

string WeapSettings::writeWeapon()
{
	if(written)
		return writtenName;

	writtenName = string(name) + ".wpn";
	filterName(writtenName);
	written = true;

	fs::path p(tc->dest / "weapons" / writtenName);
	createFolders(p);
	fs::ofstream f(p);
	
	f << setprecision(3);
	f << "reload_time = " << time(loadingtime) << endl;
	f << "ammo = " << (int)ammo << endl;
	f << "on primary_press" << endl;
	if(delay > 0)
		f << "  delay(" << time(delay) << ")" << endl;
	if(launchsound > 0)
		f << "  play_sound_static(" << tc->s[ref(launchsound)].writeSound() << ")" << endl;
	
	double motionInheritance = 0.0;
	if(affectbywormspeed)
	{
		if(speed < 100) speed = 100;
		motionInheritance = 100.0 / (double)speed;
	}
	
	double angleVariation = 0.0;
		
	f << "  shoot_particles(" << writeWObj() << ", " << (int)parts << ", "
		<< velocityFactor(speed) << ", 0, " << motionInheritance << ", 0, " << angleVariation << ", 0)" << endl;

	return writtenName;
}

string WeapSettings::writeWObj()
{
	if(objWritten)
		return objWrittenName;

	objWrittenName = string(name) + ".obj";
	filterName(objWrittenName);
	objWritten = true;

	fs::path p(tc->dest / "objects" / objWrittenName);
	createFolders(p);
	fs::ofstream f(p);
	
	//ObjGovernor g(f);
	f << setprecision(3);
	f << "gravity = " << acceleration(gravity) << endl;
	f << "acceleration = " << accelerationRatio(addspeed) << endl;
	f << "bounce_factor = " << factor(bounce) << endl;
	//cout << "sprite" << writeAnimation() << endl; //TODO
	f << "anim_duration = " << time(8) << endl;
	f << "anim_type = loop_right" << endl; //TODO: We need the animation to depend on the direction of the object
	//TODO: Angled sprites

	if(startframe < 0)
	{
		RGB& color = tc->palette[colorbullets];
		f << "colour = " << color.r << ' ' << color.g << ' ' << color.b << endl;
	}

	if(idx == 29) //Laser
		f << "repeat = 10000" << endl; //TODO: Fix a bug in vermes, objects keep getting processed after they get destroyed
	else if(shottype == SHOTTYPE_LASER)
		f << "repeat = 8" << endl;

	if(multspeed != 100)
		f << "damping = " << factor(multspeed) << endl; //WRONG!

	if(explground)
	{
		f << "on ground_collision" << endl;
		writeWObjExplActions(f);
	}

	if(timetoexplosion > 0)
	{
		f << "on timer " << time(timetoexplosion) << ' ' << time(timetoexplosionv) << endl;
		writeWObjExplActions(f);
	}

	if(parttrailobj != 0)
	{
		ObjSettings& obj = tc->o[ref(parttrailobj)];

		f << "on timer " << time(parttraildelay) << endl;
		f << "  shoot_particles(" << obj.writeObj() << ", 1, 0, 0, 0.333, 0, 0, 0)" << endl;
	}

	
	return objWrittenName;
}

void ObjSettings::writeObjExplActions(std::ostream& f)
{
	//TODO: Call writeObj() with a desired color
	if(splinteramount > 0 && splintertype != 0) //TODO: splinterscatter changes the angle variation
	{
		double motionInheritance = 0.0;
		double angleVariation = 0.0;
		f << "  shoot_particles(" << tc->o[ref(splintertype)].writeObj() << ", " << (int)splinteramount << ", "
		<< velocityFactor(tc->o[ref(splintertype)].speed) << ", " << velocityFactor(tc->o[ref(splintertype)].speedv) <<
		", " << motionInheritance << ", 0, " << angleVariation << ", 0)" << endl;
	}
	f << "  remove()" << endl;
	//TODO: createonexp and dirteffect
	
}

string ObjSettings::writeObj()
{
	if(written)
		return writtenName;

	stringstream ss;
	ss << "obj" << idx << ".obj";
	ss >> writtenName;
	filterName(writtenName);
	written = true;

	fs::path p(tc->dest / "objects" / writtenName);
	createFolders(p);
	fs::ofstream f(p);

	f << setprecision(3);
	f << "gravity = " << acceleration(gravity) << endl;
	f << "bounce_factor = " << factor(bounce) << endl;
	//cout << "sprite" << writeAnimation() << endl; //TODO
	f << "anim_duration = " << (int)time(8) << endl;
	f << "anim_type = loop_right" << endl; //TODO: We need the animation to depend on the direction of the object
	
	if(startframe <= 0)
	{
		RGB& color = tc->palette[colorbullets];
		f << "colour = " << color.r << ' ' << color.g << ' ' << color.b << endl;
	}

	if(explground)
	{
		f << "on ground_collision" << endl;
		writeObjExplActions(f);
	}

	if(timetoexplosion > 0)
	{
		f << "on timer " << time(timetoexplosion) << ' ' << time(timetoexplosionv) << endl;
		writeObjExplActions(f);
	}

	if(bloodtrail)
	{
	/* Write a secondary blood object
		ObjSettings& obj = tc->o[ref(7)];

		f << "on timer " << time(bloodtraildelay) << endl;
		f << "  shoot_particles(" << obj.writeObj() << ", 1, 0, 0, 0.333, 0, <angle variation>, 0)" << endl;*/
	}

	return writtenName;
}

void outputC(ostream& out, istream& in)
{
	out << "static char const moo[] = {";

	for(int i = 0;; ++i)
	{
		char c;
		if(!in.get(c))
		{
			out << "};";
			return;
		}

		if((i % 16) == 15)
			out << endl;
		int b = (int)(unsigned char)c;
		out << b << ",";

	}
}

int baz() { struct foo { static int bar() { return 6; } }; printf("%i\n", foo::bar()); }

int main()
{
	fs::ifstream f("/home/glip/liero/liero.exe", ios::binary);
	fs::ifstream s("/home/glip/liero/liero.snd", ios::binary);

	TC tc("/usr/local/htdocs/stuff/testmod");
	tc.read(f, s);
	for(int i = 0; i < 40; ++i)
		tc.w[i].writeWeapon();

	return 0;
}