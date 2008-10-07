/*
	OpenLieroX
 
	CWorld
 
	code under LGPL
 
 */

#ifndef __WORLD_H__
#define __WORLD_H__

// TODO: there was no single comment in this file. the file also was completly broken, all includes missing, etc. is this file actually used somewhere?

class CMap;
class CWorm;
class CProjectile;
class CBonus;

// TODO: more are missing here...


class CWorld  {
public:
	CWorld(CMap *m, CWorm *w, CProjectile *p) :
		map(m),
		worms(w),
		projectiles(p) {}

private:
	CMap *map;

	size_t topWorm;
	CWorm *worms;

	size_t topProjectile;
	CProjectile *projectiles;

	size_t topBonus;
	CBonus *bonuses;

private:
	void DrawProjectileShadows(SDL_Surface *bmpDest, CViewport *v);
	void DrawWormShadows(SDL_Surface *bmpDest);
	void DrawProjectiles(SDL_Surface *bmpDest);
	void DrawWorms(SDL_Surface *bmpDest);

public:
	void Initialize();
	void Shutdown();

	void StartSimulation()  { bSimulating = true; }
	void StopSimulation() { bSimulating = false; }

	void Draw(SDL_Surface *bmpDest, CViewport *v);
	void Simulate(float dt);

	// Projectile related functions
	void SpawnProjectile(CVec pos, CVec vel, int rot, int owner, proj_t *_proj, int _random, float remotetime, float ignoreWormCollBeforeTime);
	void DestroyProjectile(int id);

	// Worm related functions
	void SpawnWorm(int id, CVec pos);
	void KillWorm(int id);
	void RemoveWorm(int id);

	CMap *getMap() { return map; }
	void LoadMap(const std::string& name);
};

#endif // __WORLD_H__
