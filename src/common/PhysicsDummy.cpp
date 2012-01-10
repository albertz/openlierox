#include "PhysicsDummy.h"


struct DummyPhysicsEngine : PhysicsEngine {
	std::string m_engineName;
	bool m_inited;
	DummyPhysicsEngine(const std::string& e) : m_engineName(e), m_inited(false) {}

	virtual std::string name() { return m_engineName; }

	virtual void initGame() { m_inited = true; }
	virtual void uninitGame() { m_inited = false; }
	virtual bool isInitialised() { return m_inited; }

	virtual void simulateWorm(CWorm* worm, bool local) {}
	virtual void simulateWormWeapon(CWorm* worm) {}
	virtual void simulateProjectiles(Iterator<CProjectile*>::Ref projs) {}
	virtual void simulateBonuses(CBonus* bonuses, size_t count) {}

};

PhysicsEngine* CreatePhysicsEngineDummy(const std::string& enginename) {
	return new DummyPhysicsEngine(enginename);
}
