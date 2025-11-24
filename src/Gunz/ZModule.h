#pragma once

#include <vector>
#include <unordered_map>
#include "MRTTI.h"

class ZModuleContainer;

class ZModule {
public:
	virtual ~ZModule() = default;
	void Update(float Elapsed) { if (Active) OnUpdate(Elapsed); }
	virtual void InitStatus() {}
	virtual void OnAdd() {}  // Llamado después de agregar el módulo al contenedor

	ZModuleContainer* m_pContainer;
	bool Active = false;

private:
	virtual void OnUpdate(float Elapsed) {}
};

class ZModuleContainer {
	MDeclareRootRTTI;
public:
	virtual ~ZModuleContainer() = default;

	template <typename T>
	T* AddModule()
	{
#ifdef _DEBUG
		auto it = Modules.find(T::ID);
		if (it != Modules.end())
		{
			assert(false && "Module already exists");
			return nullptr;
		}
#endif

		auto ret = Modules.emplace(T::ID, std::make_unique<T>());

		if (!ret.second)
		{
			// Módulo ya existe o fallo al insertar
#ifdef _DEBUG
			_ASSERT(false && "ZModuleContainer::AddModule: Failed to add module (duplicate or insertion failed)");
#endif
			return nullptr;
		}

		auto* Module = static_cast<T*>(ret.first->second.get());

		Module->m_pContainer = this;

		// Llamar OnAdd() después de establecer el contenedor
		// OnAdd() es virtual y opcional, algunos módulos pueden no implementarlo
		Module->OnAdd();

		return Module;
	}

	ZModule* GetModule(int nID);

	void ActivateModule(int nID, bool bActive = true);
	bool IsActiveModule(int nID);

	virtual void UpdateModules(float fElapsed);
	virtual void InitModuleStatus(void);

private:
	friend ZModule;

	std::unordered_map<int, std::unique_ptr<ZModule>> Modules;
};

#define DECLARE_ID(_ID) static constexpr int ID = _ID;

class ZModule_HPAP;
class ZModule_Movable;
class ZModule_Resistance;
struct ZModule_ColdDamage;
struct ZModule_FireDamage;
struct ZModule_PoisonDamage;
struct ZModule_LightningDamage;
class ZModule_Skills;