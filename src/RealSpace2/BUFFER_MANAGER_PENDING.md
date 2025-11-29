# Buffer Manager - Lugares Pendientes

## ✅ Fase 1: COMPLETADA
- RVertexBuffer / RIndexBuffer (RMeshUtil.cpp) - ✅ COMPLETADO

## ⚠️ Fase 2: PENDIENTE (Alto Impacto)

### 1. **RBspObject::CreateVertexBuffer()** 
- **Ubicación**: `src/RealSpace2/Source/RBspObject.cpp:2198`
- **Tipo**: Buffer estático grande del mapa
- **Pool**: `D3DPOOL_MANAGED`
- **Uso**: `D3DUSAGE_WRITEONLY` (estático)
- **Viabilidad**: ✅ **ALTA** - Buffer grande, se beneficiaría mucho del pooling
- **Impacto**: Alto - se crea al cargar cada mapa

### 2. **RBspObject::CreateIndexBuffer()**
- **Ubicación**: `src/RealSpace2/Source/RBspObject.cpp:1066`
- **Tipo**: Buffer estático grande del mapa
- **Pool**: `D3DPOOL_MANAGED`
- **Uso**: `D3DUSAGE_WRITEONLY` (estático)
- **Viabilidad**: ✅ **ALTA** - Buffer grande, se beneficiaría mucho del pooling
- **Impacto**: Alto - se crea al cargar cada mapa

### 3. **RBspObjectDrawD3D9::CreateBuffers()**
- **Ubicación**: `src/RealSpace2/Source/RBspObjectDrawD3D9.cpp:128`
- **Tipo**: Múltiples vertex buffers + index buffer
- **Pool**: `D3DPOOL_MANAGED`
- **Uso**: Estático (sin D3DUSAGE_DYNAMIC)
- **Viabilidad**: ✅ **ALTA** - Múltiples buffers grandes
- **Impacto**: Alto - se crea al cargar cada mapa con iluminación dinámica

## ⚠️ Fase 3: NO RECOMENDADO (Bajo Impacto)

### 4. **RBspObject::CreateDynamicLightVertexBuffer()**
- **Ubicación**: `src/RealSpace2/Source/RBspObject.cpp:2664`
- **Tipo**: Buffer dinámico
- **Pool**: `D3DPOOL_DEFAULT`
- **Uso**: `D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY`
- **Viabilidad**: ❌ **NO** - Buffer dinámico, no se beneficia del pooling

### 5. **RCharCloth::OnRestore()**
- **Ubicación**: `src/RealSpace2/Source/RCharCloth.cpp:737`
- **Tipo**: Buffer dinámico para ropa
- **Pool**: `D3DPOOL_DEFAULT`
- **Uso**: `D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY`
- **Viabilidad**: ❌ **BAJA** - Buffer dinámico, menos beneficios

### 6. **RParticleSystem**
- **Ubicación**: `src/RealSpace2/Source/RParticleSystem.cpp:206`
- **Tipo**: Buffer dinámico para partículas
- **Viabilidad**: ❌ **BAJA** - Buffer dinámico, menos beneficios

### 7. **RRoam**
- **Ubicación**: `src/RealSpace2/Source/RRoam.cpp:540`
- **Tipo**: Buffers dinámicos para terreno
- **Pool**: `D3DPOOL_DEFAULT`
- **Uso**: `D3DUSAGE_DYNAMIC`
- **Viabilidad**: ❌ **BAJA** - Buffers dinámicos, menos beneficios

## Recomendación

**Implementar Fase 2** para obtener beneficios adicionales en buffers grandes del mapa:
- RBspObject::CreateVertexBuffer()
- RBspObject::CreateIndexBuffer()
- RBspObjectDrawD3D9::CreateBuffers()

Estos son buffers grandes que se crean al cargar mapas, por lo que se beneficiarían mucho del pooling.



