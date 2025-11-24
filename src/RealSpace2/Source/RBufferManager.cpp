#include "stdafx.h"
#include "RBufferManager.h"
#include "RealSpace2.h"
#include "MDebug.h"

_NAMESPACE_REALSPACE2_BEGIN

RBufferManager::~RBufferManager()
{
	OnInvalidate();
}

D3DPOOL RBufferManager::GetOptimalPool() const
{
	// ESTRATEGIA DE POOL:
	// - D3DPOOL_MANAGED: Más seguro, funciona con DX9 y DX9Ex
	//   - Ventaja: No requiere manejo manual de pérdida de dispositivo
	//   - Desventaja: Copia automática a VRAM (más lento)
	//   - Uso: Buffers pequeños, buffers dinámicos, compatibilidad máxima
	//
	// - D3DPOOL_DEFAULT: Más rápido, requiere DX9Ex para uso seguro
	//   - Ventaja: Acceso directo a VRAM (mejor rendimiento)
	//   - Desventaja: Requiere liberar/recrear en pérdida de dispositivo
	//   - Uso: Buffers estáticos grandes (solo con DX9Ex)
	//
	// Por ahora, usar D3DPOOL_MANAGED para compatibilidad total con DX9 y DX9Ex
	// En el futuro, se puede optimizar para usar D3DPOOL_DEFAULT con DX9Ex
	// para buffers estáticos grandes (>1MB) que no cambian frecuentemente
	
	// TODO: Implementar lógica más inteligente:
	// if (GetDX9Ex() && Size > 1024*1024 && !(Usage & D3DUSAGE_DYNAMIC))
	//     return D3DPOOL_DEFAULT;  // Buffer estático grande con DX9Ex
	
	return D3DPOOL_MANAGED;  // Compatible con DX9 y DX9Ex, seguro y funcional
}

LPDIRECT3DVERTEXBUFFER9 RBufferManager::CreateNewVertexBuffer(size_t Size, u32 FVF, u32 Usage)
{
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	D3DPOOL Pool = GetOptimalPool();
	
	HRESULT hr = RGetDevice()->CreateVertexBuffer(
		Size, Usage, FVF, Pool, &pVB, nullptr);
	
	if (FAILED(hr))
	{
		mlog("RBufferManager::CreateNewVertexBuffer failed (hr=0x%08X, Size=%zu)\n", hr, Size);
		return nullptr;
	}
	
	m_TotalMemory += Size;
	return pVB;
}

LPDIRECT3DINDEXBUFFER9 RBufferManager::CreateNewIndexBuffer(size_t Size, D3DFORMAT Format, u32 Usage)
{
	LPDIRECT3DINDEXBUFFER9 pIB = nullptr;
	D3DPOOL Pool = GetOptimalPool();
	
	HRESULT hr = RGetDevice()->CreateIndexBuffer(
		Size, Usage, Format, Pool, &pIB, nullptr);
	
	if (FAILED(hr))
	{
		mlog("RBufferManager::CreateNewIndexBuffer failed (hr=0x%08X, Size=%zu)\n", hr, Size);
		return nullptr;
	}
	
	m_TotalMemory += Size;
	return pIB;
}

LPDIRECT3DVERTEXBUFFER9 RBufferManager::GetVertexBuffer(size_t Size, u32 FVF, u32 Usage)
{
	m_CurrentFrame = g_nFrameCount;
	
	BufferKey key;
	key.Size = Size;
	key.FVF = FVF;
	key.IndexFormat = D3DFMT_UNKNOWN; // No aplica para vertex buffers
	
	// Buscar en el pool de buffers disponibles
	auto& pool = m_BufferPool[key];
	
	for (auto it = pool.begin(); it != pool.end(); ++it)
	{
		if (!it->bInUse && it->Size == Size && it->FVF == FVF)
		{
			// Reutilizar buffer existente
			it->bInUse = true;
			it->LastUsedFrame = m_CurrentFrame;
			LPDIRECT3DVERTEXBUFFER9 pVB = it->pVB;
			
			// Mover a lista activa
			BufferInfo info = *it;
			pool.erase(it);
			m_ActiveVBuffers[pVB] = info;
			
			return pVB;
		}
	}
	
	// No hay buffer disponible, crear uno nuevo
	LPDIRECT3DVERTEXBUFFER9 pVB = CreateNewVertexBuffer(Size, FVF, Usage);
	if (pVB)
	{
		BufferInfo info;
		info.pVB = pVB;
		info.pIB = nullptr;
		info.Size = Size;
		info.FVF = FVF;
		info.IndexFormat = D3DFMT_UNKNOWN;
		info.bInUse = true;
		info.LastUsedFrame = m_CurrentFrame;
		m_ActiveVBuffers[pVB] = info;
	}
	
	return pVB;
}

LPDIRECT3DINDEXBUFFER9 RBufferManager::GetIndexBuffer(size_t Size, D3DFORMAT Format, u32 Usage)
{
	m_CurrentFrame = g_nFrameCount;
	
	BufferKey key;
	key.Size = Size;
	key.FVF = 0; // No aplica para index buffers
	key.IndexFormat = Format;
	
	// Buscar en el pool de buffers disponibles
	auto& pool = m_BufferPool[key];
	
	for (auto it = pool.begin(); it != pool.end(); ++it)
	{
		if (!it->bInUse && it->Size == Size && it->IndexFormat == Format)
		{
			// Reutilizar buffer existente
			it->bInUse = true;
			it->LastUsedFrame = m_CurrentFrame;
			LPDIRECT3DINDEXBUFFER9 pIB = it->pIB;
			
			// Mover a lista activa
			BufferInfo info = *it;
			pool.erase(it);
			m_ActiveIBuffers[pIB] = info;
			
			return pIB;
		}
	}
	
	// No hay buffer disponible, crear uno nuevo
	LPDIRECT3DINDEXBUFFER9 pIB = CreateNewIndexBuffer(Size, Format, Usage);
	if (pIB)
	{
		BufferInfo info;
		info.pVB = nullptr;
		info.pIB = pIB;
		info.Size = Size;
		info.FVF = 0;
		info.IndexFormat = Format;
		info.bInUse = true;
		info.LastUsedFrame = m_CurrentFrame;
		m_ActiveIBuffers[pIB] = info;
	}
	
	return pIB;
}

void RBufferManager::ReleaseVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVB)
{
	if (!pVB) return;
	
	auto it = m_ActiveVBuffers.find(pVB);
	if (it != m_ActiveVBuffers.end())
	{
		BufferInfo info = it->second;
		info.bInUse = false;
		info.LastUsedFrame = m_CurrentFrame;
		
		// Mover de vuelta al pool
		BufferKey key;
		key.Size = info.Size;
		key.FVF = info.FVF;
		key.IndexFormat = D3DFMT_UNKNOWN;
		m_BufferPool[key].push_back(info);
		
		m_ActiveVBuffers.erase(it);
	}
}

void RBufferManager::ReleaseIndexBuffer(LPDIRECT3DINDEXBUFFER9 pIB)
{
	if (!pIB) return;
	
	auto it = m_ActiveIBuffers.find(pIB);
	if (it != m_ActiveIBuffers.end())
	{
		BufferInfo info = it->second;
		info.bInUse = false;
		info.LastUsedFrame = m_CurrentFrame;
		
		// Mover de vuelta al pool
		BufferKey key;
		key.Size = info.Size;
		key.FVF = 0;
		key.IndexFormat = info.IndexFormat;
		m_BufferPool[key].push_back(info);
		
		m_ActiveIBuffers.erase(it);
	}
}

void RBufferManager::CleanupUnusedBuffers(DWORD CurrentFrame, DWORD MaxAge)
{
	m_CurrentFrame = CurrentFrame;
	
	// OPTIMIZACIÓN: Solo limpiar si hay suficientes buffers en el pool
	// Esto evita iterar sobre pools pequeños innecesariamente
	size_t totalPoolSize = 0;
	for (auto& pair : m_BufferPool)
		totalPoolSize += pair.second.size();
	
	// Si el pool es pequeño (<100 buffers), no vale la pena limpiar frecuentemente
	// Solo limpiar si hay muchos buffers o si han pasado muchos frames
	if (totalPoolSize < 100 && (CurrentFrame % (MaxAge * 4)) != 0)
		return;
	
	size_t buffersFreed = 0;
	size_t memoryFreed = 0;
	size_t poolsProcessed = 0;
	const size_t MAX_POOLS_PER_CLEANUP = 50;  // Limitar pools procesados por iteración
	
	// Limpiar buffers no usados del pool (procesar solo una fracción cada vez)
	for (auto& pair : m_BufferPool)
	{
		if (poolsProcessed >= MAX_POOLS_PER_CLEANUP)
			break;  // Limpieza incremental: procesar solo algunos pools por vez
		
		auto& pool = pair.second;
		if (pool.empty())
			continue;
		
		poolsProcessed++;
		
		for (auto it = pool.begin(); it != pool.end();)
		{
			if (!it->bInUse && (CurrentFrame - it->LastUsedFrame) > MaxAge)
			{
				// Liberar buffer antiguo
				if (it->pVB)
				{
					SAFE_RELEASE(it->pVB);
					memoryFreed += it->Size;
					buffersFreed++;
				}
				if (it->pIB)
				{
					SAFE_RELEASE(it->pIB);
					memoryFreed += it->Size;
					buffersFreed++;
				}
				it = pool.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

void RBufferManager::OnInvalidate()
{
	// Con D3DPOOL_MANAGED, los buffers se invalidan automáticamente
	// pero DirectX los restaura automáticamente también
	// Solo necesitamos limpiar referencias, no liberar memoria
	
	// Limpiar referencias del pool (los buffers se restaurarán automáticamente)
	for (auto& pair : m_BufferPool)
	{
		auto& pool = pair.second;
		// Con D3DPOOL_MANAGED, no liberamos, solo marcamos como no disponibles
		// Los buffers se restaurarán automáticamente cuando se necesiten
		for (auto& info : pool)
		{
			info.bInUse = false;  // Marcar como disponible para limpieza
		}
	}
	
	// Los buffers activos mantendrán sus referencias
	// Con D3DPOOL_MANAGED, DirectX los restaurará automáticamente
}

void RBufferManager::OnRestore()
{
	// Con D3DPOOL_MANAGED, los buffers se restauran automáticamente
	// No necesitamos hacer nada especial aquí
	// Los buffers se recrearán cuando se soliciten si es necesario
	
	// Si usáramos D3DPOOL_DEFAULT, necesitaríamos recrear todos los buffers aquí
	// pero como usamos D3DPOOL_MANAGED, DirectX lo hace automáticamente
}

_NAMESPACE_REALSPACE2_END

