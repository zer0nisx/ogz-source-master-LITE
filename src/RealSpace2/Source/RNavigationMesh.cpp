#include "stdafx.h"
#include "RNavigationMesh.h"
#include "RNavigationNode.h"
#include "MZFileSystem.h"
#include "RVersions.h"
#include "RMath.h"

// MEJORA: Definir umbral de distancia para caché
const float RNavigationMesh::CACHE_DISTANCE_THRESHOLD = 50.0f;

RNavigationMesh::RNavigationMesh()
{
	m_nVertCount = 0;
	m_nFaceCount = 0;
	m_vertices = NULL;
	m_faces = NULL;
	m_pLastFoundNode = NULL;
	m_LastSearchPoint = rvector(0, 0, 0);
}

RNavigationMesh::~RNavigationMesh()
{
	Clear();
}

void RNavigationMesh::Clear()
{
	if (m_vertices != NULL) delete [] m_vertices;
	if (m_faces != NULL) delete [] m_faces;

	m_nVertCount = 0;
	m_nFaceCount = 0;
	m_vertices = NULL;
	m_faces = NULL;

	// MEJORA: Limpiar caché al limpiar el mesh
	m_pLastFoundNode = NULL;
	m_LastSearchPoint = rvector(0, 0, 0);

	ClearNodes();
}

void RNavigationMesh::ClearNodes()
{
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		delete (*itor);
	}
	m_NodeArray.clear();
}

void RNavigationMesh::AddNode(int nID, const rvector& PointA, const rvector& PointB, const rvector& PointC)
{
	RNavigationNode* pNewNode = new RNavigationNode;

	pNewNode->Init(nID, PointA, PointB, PointC);
	m_NodeArray.push_back(pNewNode);
}

void RNavigationMesh::MakeNodes()
{
	if (!m_NodeArray.empty()) ClearNodes();

	for (int i = 0; i < m_nFaceCount; i++)
	{
		rvector* vp[3];
		vp[0] = &m_vertices[m_faces[i].v1];
		vp[1] = &m_vertices[m_faces[i].v2];
		vp[2] = &m_vertices[m_faces[i].v3];

		rvector temp = m_vertices[m_faces[i].v3];

		_ASSERT( (*vp[0] != *vp[1]) && (*vp[1] != *vp[2]) && (*vp[2] != *vp[1]) );
		AddNode(i, *vp[0], *vp[1], *vp[2]);		// �ݽð����
		//AddNode(i, *vp[0], *vp[2], *vp[1]);		// �ð����
	}
}

void RNavigationMesh::LinkNodes()
{
	for (RNodeArray::iterator itorA = m_NodeArray.begin(); itorA != m_NodeArray.end(); ++itorA)
	{
		RNavigationNode* pNodeA = (*itorA);
		for (RNodeArray::iterator itorB = m_NodeArray.begin(); itorB != m_NodeArray.end(); ++itorB)
		{
			if (itorA == itorB) continue;

			RNavigationNode* pNodeB = (*itorB);

			if (!pNodeA->GetLink(RNavigationNode::SIDE_AB) && pNodeB->RequestLink(pNodeA->Vertex(0), pNodeA->Vertex(1), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_AB, pNodeB);
			}
			else if (!pNodeA->GetLink(RNavigationNode::SIDE_BC) && pNodeB->RequestLink(pNodeA->Vertex(1), pNodeA->Vertex(2), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_BC, pNodeB);
			}
			else if (!pNodeA->GetLink(RNavigationNode::SIDE_CA) && pNodeB->RequestLink(pNodeA->Vertex(2), pNodeA->Vertex(0), pNodeA))
			{
				pNodeA->SetLink(RNavigationNode::SIDE_CA, pNodeB);
			}
		}
	}
}

RNavigationNode* RNavigationMesh::FindClosestNode(const rvector& point) const
{
	// MEJORA: Verificar caché primero
	if (m_pLastFoundNode != NULL)
	{
		float cacheDistSq = MagnitudeSq(point - m_LastSearchPoint);
		if (cacheDistSq < CACHE_DISTANCE_THRESHOLD * CACHE_DISTANCE_THRESHOLD)
		{
			// Verificar si el punto sigue dentro del nodo en caché
			if (m_pLastFoundNode->IsPointInNodeColumn(point))
			{
				return m_pLastFoundNode;
			}
		}
	}

	float ClosestDistance = FLT_MAX;
	float ClosestHeight = FLT_MAX;
	bool bFoundHomeNode = false;
	float ThisDistance;
	RNavigationNode* pClosestNode=NULL;

		
	for (RNodeArray::const_iterator itorNode = m_NodeArray.begin(); itorNode != m_NodeArray.end(); ++itorNode)
	{
		RNavigationNode* pNode = (*itorNode);

		if (pNode->IsPointInNodeColumn(point))
		{
			rvector NewPosition(point);
			pNode->MapVectorHeightToNode(NewPosition);

			// ���� ����� ������ ��带 ã�´�.
			ThisDistance = fabs(NewPosition.z - point.z);

			if (bFoundHomeNode)
			{
				if (ThisDistance < ClosestHeight)
				{
					pClosestNode = pNode;
					ClosestHeight = ThisDistance;
				}
			}
			else
			{
				pClosestNode = pNode;
				ClosestHeight = ThisDistance;
				bFoundHomeNode = true;
			}
		}

		if (!bFoundHomeNode)
		{
			rvector2 Start(pNode->CenterVertex().x, pNode->CenterVertex().y);
			rvector2 End(point.x, point.y);
			rline2d MotionPath(Start, End);

			RNavigationNode* pNextNode;
			RNavigationNode::NODE_SIDE WallHit;
			rvector2 PointOfIntersection;

			RNavigationNode::PATH_RESULT Result = pNode->ClassifyPathToNode(MotionPath, &pNextNode, WallHit, &PointOfIntersection);

			if (Result == RNavigationNode::EXITING_NODE)
			{
				rvector ClosestPoint3D(PointOfIntersection.x, PointOfIntersection.y, 0.0f);
				pNode->MapVectorHeightToNode(ClosestPoint3D);

				ClosestPoint3D -= point;

				ThisDistance = Magnitude(ClosestPoint3D);

				if (ThisDistance<ClosestDistance)
				{
					ClosestDistance=ThisDistance;
					pClosestNode = pNode;
				}
			}
		}
	}
	
	// MEJORA: Actualizar caché con el resultado
	if (pClosestNode != NULL)
	{
		m_pLastFoundNode = pClosestNode;
		m_LastSearchPoint = point;
	}

	return pClosestNode;
}

#ifdef _WIN32
void DrawNavFace(LPDIRECT3DDEVICE9 pd3dDevice, rvector* vertices, int v1, int v2, int v3)
{
	rvector v[3];
	v[0] = vertices[v1];
	v[1] = vertices[v2];
	v[2] = vertices[v3];

	pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, v, sizeof(rvector));
}

void DrawNavFaceWireFrame(LPDIRECT3DDEVICE9 pd3dDevice, rvector* vertices, int v1, int v2, int v3)
{
	rvector v[4];
	v[0] = vertices[v1];
	v[1] = vertices[v2];
	v[2] = vertices[v3];
	v[3] = vertices[v1];

	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP,3,&v,sizeof(rvector));
}

#include "RealSpace2.h"
using namespace RealSpace2;

void RNavigationMesh::Render()
{
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();

	RSetTransform(D3DTS_WORLD, GetIdentityMatrix());

	pd3dDevice->SetFVF( D3DFVF_XYZ );
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW );
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FF9100);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	RSetWBuffer(true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40ffffff);

	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = *itor;

		if (pNode->bSelected) pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FFFFFF);
		else pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40FF9100);

		rvector _vertices[3];
		_vertices[0] = pNode->Vertex(0);
		_vertices[1] = pNode->Vertex(1);
		_vertices[2] = pNode->Vertex(2);

		DrawNavFaceWireFrame(pd3dDevice, _vertices, 0, 1, 2);

	}


	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0x40ff00ff);

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

}


void RNavigationMesh::RenderLinks()
{
	RSetTransform(D3DTS_WORLD, GetIdentityMatrix());

	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR , 0xFF0033FF);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	DWORD color = 0xFFFF0000;

	for (RNavigationMesh::RNodeArray::iterator itor = GetNodes()->begin();
		itor != GetNodes()->end(); ++itor)
	{
		RNavigationNode* pNodeA = (*itor);

		if (pNodeA->GetLink(RNavigationNode::SIDE_AB))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_AB);

			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}


		if (pNodeA->GetLink(RNavigationNode::SIDE_BC))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_BC);

			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}

		if (pNodeA->GetLink(RNavigationNode::SIDE_CA))
		{
			RNavigationNode* pSideNode = pNodeA->GetLink(RNavigationNode::SIDE_CA);
			rvector p1, p2;
			p1 = pNodeA->CenterVertex();
			p2 = pSideNode->CenterVertex();

			RDrawLine(p1, p2, color);
		}

	}

}
#endif

rvector RNavigationMesh::SnapPointToNode(RNavigationNode* pNode, const rvector& Point)
{
	rvector PointOut = Point;

	//if (!pNode->IsPointInNodeColumn(PointOut))
	{
		pNode->ForcePointToNodeColumn(PointOut);
	}

	pNode->MapVectorHeightToNode(PointOut);
	return (PointOut);
}

rvector RNavigationMesh::SnapPointToMesh(RNavigationNode** NodeOut, const rvector& Point)
{
	rvector PointOut = Point;
	*NodeOut = FindClosestNode(PointOut);
	return (SnapPointToNode(*NodeOut, PointOut));
}

bool RNavigationMesh::BuildNavigationPath(RNavigationNode* pStartNode, 
							const rvector& StartPos, RNavigationNode* pEndNode, const rvector& EndPos)
{
	m_pStartNode = pStartNode;
	m_pGoalNode = pEndNode;

	bool ret = m_AStar.Search(pStartNode, pEndNode);
	if (ret == false) 
	{
		#ifdef _DEBUG
		OutputDebugString("RNavigationMesh::BuildNavigationPath - A* search failed\n");
		#endif
		return false;
	}

	m_WaypointList.clear();
	// MEJORA: Validar altura del waypoint final usando el nodo
	rvector validatedEndPos = EndPos;
	if (pEndNode)
	{
		validatedEndPos = SnapPointToNode(pEndNode, EndPos);
	}
	m_WaypointList.push_back(validatedEndPos);


	RNavigationNode* pVantageNode = NULL;
	rvector vantagePos;

	pVantageNode = pEndNode;
	vantagePos = EndPos;

	RNavigationNode* pLastNode = NULL;
	rvector lastPos;

	list<RAStarNode*>* pPath = &m_AStar.m_ShortestPath;

	bool bPushed = true;
	for (list<RAStarNode*>::iterator itor = pPath->begin(); itor != pPath->end(); itor++)
	{
		RNavigationNode* pTestNode = (RNavigationNode*)(*itor);

		rvector testPos = pTestNode->GetWallMidPoint(pTestNode->GetArrivalLink());
		testPos = SnapPointToNode(pTestNode, testPos);

		if (LineOfSightTest(pVantageNode, vantagePos, pTestNode, testPos))
		{
			// MEJORA: Aunque hay línea de vista, agregar waypoint intermedio si:
			// 1. La distancia horizontal es grande (>500 unidades)
			// 2. Hay cambio significativo de altura (>30 unidades) - para escaleras y relieve
			rvector diff = testPos - vantagePos;
			float distHorizontal = Magnitude(rvector(diff.x, diff.y, 0.0f));
			float heightDiff = fabs(diff.z);
			
			// MEJORA: Agregar waypoint intermedio si hay cambio significativo de altura (escaleras/relieve)
			// o si la distancia horizontal es grande, PERO solo si la distancia total es suficiente
			// Evitar generar waypoints en distancias muy cortas
			const float MIN_DISTANCE_FOR_WAYPOINT = 200.0f;  // Distancia mínima para generar waypoint (aumentado de 100 a 200)
			bool needsIntermediate = false;
			
			if (heightDiff > 50.0f && distHorizontal > MIN_DISTANCE_FOR_WAYPOINT)  // Cambio de altura significativo (50 unidades) y distancia suficiente (200 unidades)
			{
				needsIntermediate = true;
				// Para escaleras, agregar múltiples waypoints intermedios solo si la distancia es suficiente
				int numHeightWaypoints = (int)(heightDiff / 40.0f);  // Cada 40 unidades de altura (menos frecuente)
				// Solo agregar waypoints intermedios si la distancia horizontal justifica múltiples waypoints
				if (numHeightWaypoints > 1 && distHorizontal > 300.0f && pLastNode != NULL)
				{
					for (int i = 1; i < numHeightWaypoints; i++)
					{
						float t = (float)i / numHeightWaypoints;
						rvector intermediatePos = vantagePos + diff * t;
						intermediatePos = SnapPointToNode(pLastNode, intermediatePos);
						
						// Validar que el waypoint intermedio no esté muy cerca del anterior
						if (!m_WaypointList.empty())
						{
							rvector lastWaypoint = m_WaypointList.back();
							float distToLast = Magnitude(intermediatePos - lastWaypoint);
							if (distToLast < MIN_DISTANCE_FOR_WAYPOINT)
								continue;  // Saltar si está muy cerca
						}
						
						m_WaypointList.push_back(intermediatePos);
					}
				}
			}
			else if (distHorizontal > 800.0f)  // Distancia horizontal grande (aumentado de 500 a 800 para evitar waypoints innecesarios)
			{
				needsIntermediate = true;
			}
			
			if (needsIntermediate && distHorizontal > MIN_DISTANCE_FOR_WAYPOINT && pLastNode != NULL)
			{
				// Agregar waypoint intermedio ANTES del punto final para mejor navegación
				// Solo si la distancia es suficiente
				rvector intermediatePos = (vantagePos + testPos) * 0.5f;
				intermediatePos = SnapPointToNode(pLastNode, intermediatePos);
				
				// Validar que no esté muy cerca del waypoint anterior
				if (m_WaypointList.empty() || Magnitude(intermediatePos - m_WaypointList.back()) >= MIN_DISTANCE_FOR_WAYPOINT)
				{
					m_WaypointList.push_back(intermediatePos);
				}
			}
			
mn 			pLastNode = pTestNode;
			lastPos = testPos;
			bPushed = false;
		}
		else
		{
			_ASSERT(pLastNode != NULL);
			// MEJORA: Validar altura del waypoint usando el nodo
			rvector validatedLastPos = SnapPointToNode(pLastNode, lastPos);
			m_WaypointList.push_back(validatedLastPos);
			pVantageNode = pLastNode;
			vantagePos = validatedLastPos;
			bPushed = true;
		}
	}
	
	if (!bPushed) 
	{
		if (!LineOfSightTest(pVantageNode, vantagePos, pStartNode, StartPos))
		{
			// MEJORA: Validar altura del waypoint usando el nodo
			if (pLastNode)
			{
				rvector validatedLastPos = SnapPointToNode(pLastNode, lastPos);
				m_WaypointList.push_back(validatedLastPos);
			}
			else
			{
				m_WaypointList.push_back(lastPos);
			}
		}
	}

	//m_WaypointList.push_back(StartPos);


	// ������ �ڹٲ۴�.
	m_WaypointList.reverse();

	// MEJORA: Garantizar mínimo de waypoints en rutas largas
	// Ajustado para mapas grandes de 15000+ unidades
	EnsureMinimumWaypoints(6000.0f, 8);  // Mínimo 8 waypoints en rutas >6000 unidades

	return ret;
}

bool RNavigationMesh::BuildNavigationPath(const rvector& vStartPos, const rvector& vGoalPos)
{
	RNavigationNode* pStartNode = FindClosestNode(vStartPos);
	if (pStartNode == NULL) 
	{
		#ifdef _DEBUG
		OutputDebugString("RNavigationMesh::BuildNavigationPath - No start node found\n");
		#endif
		return false;
	}

	RNavigationNode* pGoalNode = FindClosestNode(vGoalPos);
	if (pGoalNode == NULL) 
	{
		#ifdef _DEBUG
		OutputDebugString("RNavigationMesh::BuildNavigationPath - No goal node found\n");
		#endif
		return false;
	}

	return BuildNavigationPath(pStartNode, vStartPos, pGoalNode, vGoalPos);
}


bool RNavigationMesh::LineOfSightTest(RNavigationNode* pStartNode, const rvector& StartPos, RNavigationNode* pGoalNode, const rvector& EndPos)
{
	if ((pStartNode == NULL) || (pGoalNode == NULL)) return false;
	if (pStartNode == pGoalNode) return true;

	rline2d MotionPath(rvector2(StartPos.x,StartPos.y), rvector2(EndPos.x,EndPos.y));

	RNavigationNode* pNextNode = pStartNode;
	RNavigationNode::NODE_SIDE WallNumber;
	RNavigationNode::PATH_RESULT Result;

	while((Result = pNextNode->ClassifyPathToNode(MotionPath, &pNextNode, WallNumber, 0)) == RNavigationNode::EXITING_NODE)
	{
		if (!pNextNode) return(false);
	}


	return (Result == RNavigationNode::ENDING_NODE);
}

bool RNavigationMesh::Open(const char* szFileName, MZFileSystem* pZFileSystem)
{
	MZFile file;
	if(!file.Open(szFileName, pZFileSystem)) return false;

	// header -------------
	RHEADER header;
	file.Read(&header,sizeof(RHEADER));
	if(header.dwID!=R_NAV_ID || header.dwVersion!=R_NAV_VERSION)
	{
		file.Close();
		return false;
	}

	int nVertCount,nFaceCount;

	// vertex -------------
	file.Read(&nVertCount,sizeof(int));
	InitVertices(nVertCount);
	for (int i = 0; i < nVertCount; i++)
	{
		file.Read(&m_vertices[i], sizeof(rvector));
	}

	// face ---------------
	file.Read(&nFaceCount,sizeof(int));
	InitFaces(nFaceCount);
	for (int i = 0; i < nFaceCount; i++)
	{
		file.Read(&m_faces[i], sizeof(RNavFace));
	}

	MakeNodes();

	// link ---------------
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);

		for (int i = 0; i < 3; i++)
		{
			int nSideIndex = -1;

			if (file.Read(&nSideIndex, sizeof(int)))
			{
				if (nSideIndex >= 0)
				{
					pNode->SetLink(RNavigationNode::NODE_SIDE(i), m_NodeArray[nSideIndex]);
				}
			}
			else
			{
				_ASSERT(0);
			}
		}
	}

	file.Close();

	return true;
}

bool RNavigationMesh::Save(const char* szFileName)
{
	if (m_nVertCount <= 0) return false;

	FILE* file = fopen(szFileName, "wb");
	if (!file) return false;

	// header -------------
	RHEADER header{ R_NAV_ID, R_NAV_VERSION };
	fwrite(&header, sizeof(RHEADER), 1, file);

	// vertex -------------
	fwrite(&m_nVertCount, sizeof(int), 1, file);
	for (int i = 0; i < m_nVertCount; i++)
	{
		fwrite(&m_vertices[i], sizeof(rvector), 1, file);
	}

	// face ---------------
	fwrite(&m_nFaceCount, sizeof(int), 1, file);
	for (int i = 0; i < m_nFaceCount; i++)
	{
		fwrite(&m_faces[i], sizeof(RNavFace), 1, file);
	}

	MakeNodes();
	LinkNodes();

	// link ---------------
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);

		for (int i = 0; i < 3; i++)
		{
			int nSideIndex = -1;
			if (pNode->GetLink(i))
			{
				nSideIndex = pNode->GetLink(i)->GetID();
			}
			fwrite(&nSideIndex, sizeof(int), 1, file);
		}
	}



	fclose(file);

	return true;
}


void RNavigationMesh::ClearAllNodeWeight()
{
	for (RNodeArray::iterator itor = m_NodeArray.begin(); itor != m_NodeArray.end(); ++itor)
	{
		RNavigationNode* pNode = (*itor);
		pNode->SetWeight(1.0f);
	}
}

// MEJORA: Garantizar mínimo de waypoints en rutas largas
void RNavigationMesh::EnsureMinimumWaypoints(float minDistance, int minWaypoints)
{
	if (m_WaypointList.size() < 2) return;
	
	// Calcular distancia total
	float totalDistance = 0.0f;
	rvector prevPos = *m_WaypointList.begin();
	for (std::list<rvector>::iterator it = ++m_WaypointList.begin(); it != m_WaypointList.end(); ++it)
	{
		rvector diff = *it - prevPos;
		totalDistance += Magnitude(diff);
		prevPos = *it;
	}
	
	// Si la ruta es larga pero tiene pocos waypoints, agregar intermedios
	if (totalDistance > minDistance && m_WaypointList.size() < (size_t)minWaypoints)
	{
		std::list<rvector> newWaypoints;
		prevPos = *m_WaypointList.begin();
		newWaypoints.push_back(prevPos);
		
		for (std::list<rvector>::iterator it = ++m_WaypointList.begin(); it != m_WaypointList.end(); ++it)
		{
			rvector currentPos = *it;
			rvector segment = currentPos - prevPos;
			float segmentDist = Magnitude(segment);
			float segmentDistHorizontal = Magnitude(rvector(segment.x, segment.y, 0.0f));
			float heightDiff = fabs(segment.z);
			
			// MEJORA: Agregar waypoints intermedios si:
			// 1. El segmento horizontal es muy largo (>1000 unidades)
			// 2. Hay cambio significativo de altura (>30 unidades) - escaleras/pendientes
			// PERO solo si la distancia es suficiente para evitar waypoints innecesarios en distancias cortas
			const float MIN_DISTANCE_FOR_WAYPOINT = 200.0f;  // Distancia mínima entre waypoints (aumentado de 100 a 200)
			const float MIN_SEGMENT_FOR_INTERMEDIATES = 400.0f;  // Distancia mínima del segmento para agregar intermedios (aumentado de 200 a 400)
			
			bool needsIntermediate = false;
			int numIntermediates = 0;
			
			// Solo considerar agregar waypoints si el segmento es lo suficientemente largo
			if (segmentDistHorizontal < MIN_SEGMENT_FOR_INTERMEDIATES)
			{
				// Segmento muy corto, no agregar waypoints intermedios
				needsIntermediate = false;
			}
			else if (heightDiff > 50.0f && segmentDistHorizontal > MIN_SEGMENT_FOR_INTERMEDIATES)
			{
				// Cambio significativo de altura (escaleras/pendientes) - aumentado de 30 a 50 unidades
				// Agregar waypoints más frecuentes para seguir el relieve correctamente
				// Pero solo si la distancia horizontal justifica múltiples waypoints
				float spacing = max(200.0f, min(segmentDistHorizontal / 4.0f, heightDiff / 4.0f));  // Mínimo 200 unidades entre waypoints (aumentado)
				numIntermediates = (int)(segmentDistHorizontal / spacing);
				
				// Limitar número máximo de waypoints para evitar exceso
				if (numIntermediates > 10)  // Máximo 10 waypoints intermedios
					numIntermediates = 10;
					
				needsIntermediate = (numIntermediates > 0);
			}
			else if (segmentDistHorizontal > 1000.0f)
			{
				// Segmento largo horizontalmente
				numIntermediates = (int)(segmentDistHorizontal / 600.0f);  // Cada 600 unidades
				if (numIntermediates > 10)  // Máximo 10 waypoints intermedios
					numIntermediates = 10;
				needsIntermediate = (numIntermediates > 0);
			}
			
			if (needsIntermediate && numIntermediates > 0)
			{
				Normalize(segment);
				
				for (int i = 1; i <= numIntermediates; i++)
				{
					rvector intermediate = prevPos + segment * (segmentDist * i / (numIntermediates + 1));
					intermediate = SnapPointToMesh(nullptr, intermediate);
					
					// Validar que el waypoint intermedio no esté muy cerca del anterior
					float distToPrev = Magnitude(intermediate - newWaypoints.back());
					if (distToPrev >= MIN_DISTANCE_FOR_WAYPOINT)
					{
						newWaypoints.push_back(intermediate);
					}
				}
			}
			
			newWaypoints.push_back(currentPos);
			prevPos = currentPos;
		}
		
		m_WaypointList = newWaypoints;
	}
}