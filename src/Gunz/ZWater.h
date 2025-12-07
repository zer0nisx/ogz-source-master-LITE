#ifndef _ZWATER_H
#define _ZWATER_H

#include "RTypes.h"
#include "RMeshNode.h"
#include "MUtil.h"
#include <memory>
#include <vector>
#include <list>

using namespace RealSpace2;

enum WaterType
{
	WaterType1 = 0,
	WaterType2,
};

class ZWaterList;

class ZWater
{
	friend	ZWaterList;

protected:
	D3DPtr<IDirect3DIndexBuffer9>	m_pIndexBuffer;
	RBaseTexture* m_pTexture;
	std::vector<rvector>			m_pVerts;
	std::vector<RFaceInfo>			m_pFaces;
	int								m_nVerts;
	int								m_nFaces;

protected:
	RealSpace2::rboundingbox	m_BoundingBox;
	rmatrix			m_viewMat;
	rmatrix			m_projMat;
	rmatrix			m_worldMat;
	float			m_fbaseZpos;
	D3DMATERIAL9	m_mtrl;

public:
	int		m_nWaterType;
	bool	m_isRender;
public:
	bool SetMesh(RMeshNode* meshNode);

protected:
	void Update();
	void Render();
	void OnInvalidate();
	void OnRestore();
	bool CheckSpearing(const rvector& o, const rvector& e, int iPower, float fArea, rvector* pPos);
	bool Pick(const rvector& o, const rvector& d, rvector* pPos);
	bool RenderReflectionSurface();
	bool RenderUnderWater();
	void Ripple(const rvector& pos, int iAmplitude, float fFrequency);

public:
	ZWater(void);
	~ZWater(void);
};

class ZWaterList : public std::list<std::unique_ptr<ZWater>>
{
protected:
	DWORD	m_dwTime;

public:
	ZWaterList();
	virtual ~ZWaterList();

public:
	void Update();
	void Render();
	static void OnInvalidate();
	static void OnRestore();
	static bool SetSurface(bool b);
	void Clear();
	bool CheckSpearing(const rvector& o, const rvector& e, int iPower, float fArea, bool bPlaySound = true);
	bool Pick(rvector& o, rvector& d, rvector* pPos);
	ZWater* Get(int iIndex);

protected:
	void PreRender();
	void PostRender();
};

#endif