#pragma once

#include "RealSpace2.h"
#include <unordered_map>
#include <list>
#include <memory>

_NAMESPACE_REALSPACE2_BEGIN

// Buffer Manager para optimizar creación y reutilización de buffers
// Con DirectX 9 Extended, permite usar D3DPOOL_DEFAULT de forma más segura
class RBufferManager
{
public:
	struct BufferInfo
	{
		LPDIRECT3DVERTEXBUFFER9 pVB;
		LPDIRECT3DINDEXBUFFER9 pIB;
		size_t Size;
		u32 FVF;
		D3DFORMAT IndexFormat;
		bool bInUse;
		DWORD LastUsedFrame;
	};

	static RBufferManager& GetInstance()
	{
		static RBufferManager instance;
		return instance;
	}

	// Obtener o crear un vertex buffer
	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer(size_t Size, u32 FVF, u32 Usage = D3DUSAGE_WRITEONLY);
	
	// Obtener o crear un index buffer
	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer(size_t Size, D3DFORMAT Format = D3DFMT_INDEX16, u32 Usage = D3DUSAGE_WRITEONLY);
	
	// Liberar un buffer (lo marca como disponible para reutilización)
	void ReleaseVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVB);
	void ReleaseIndexBuffer(LPDIRECT3DINDEXBUFFER9 pIB);
	
	// Limpiar buffers no usados (llamar periódicamente)
	void CleanupUnusedBuffers(DWORD CurrentFrame, DWORD MaxAge = 300); // 300 frames = ~5 segundos a 60fps
	
	// Invalidar todos los buffers (cuando se pierde el dispositivo)
	void OnInvalidate();
	
	// Restaurar buffers (cuando se restaura el dispositivo)
	void OnRestore();
	
	// Estadísticas
	size_t GetActiveBufferCount() const { return m_ActiveVBuffers.size() + m_ActiveIBuffers.size(); }
	size_t GetTotalBufferMemory() const { return m_TotalMemory; }

private:
	RBufferManager() = default;
	~RBufferManager();
	RBufferManager(const RBufferManager&) = delete;
	RBufferManager& operator=(const RBufferManager&) = delete;

	struct BufferKey
	{
		size_t Size;
		u32 FVF;
		D3DFORMAT IndexFormat;
		
		bool operator==(const BufferKey& other) const
		{
			return Size == other.Size && FVF == other.FVF && IndexFormat == other.IndexFormat;
		}
	};

	struct BufferKeyHash
	{
		size_t operator()(const BufferKey& key) const
		{
			return std::hash<size_t>()(key.Size) ^ 
				   (std::hash<u32>()(key.FVF) << 1) ^ 
				   (std::hash<DWORD>()(key.IndexFormat) << 2);
		}
	};

	// Pool de buffers disponibles para reutilización
	std::unordered_map<BufferKey, std::list<BufferInfo>, BufferKeyHash> m_BufferPool;
	
	// Buffers activos (en uso)
	std::unordered_map<LPDIRECT3DVERTEXBUFFER9, BufferInfo> m_ActiveVBuffers;
	std::unordered_map<LPDIRECT3DINDEXBUFFER9, BufferInfo> m_ActiveIBuffers;
	
	size_t m_TotalMemory = 0;
	DWORD m_CurrentFrame = 0;
	
	// Determinar qué pool usar según DX9Ex
	D3DPOOL GetOptimalPool() const;
	
	// Crear un nuevo buffer
	LPDIRECT3DVERTEXBUFFER9 CreateNewVertexBuffer(size_t Size, u32 FVF, u32 Usage);
	LPDIRECT3DINDEXBUFFER9 CreateNewIndexBuffer(size_t Size, D3DFORMAT Format, u32 Usage);
};

_NAMESPACE_REALSPACE2_END

