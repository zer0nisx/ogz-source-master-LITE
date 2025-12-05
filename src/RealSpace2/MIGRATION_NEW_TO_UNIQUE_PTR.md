# Estrategia de Migración: `new` → `std::make_unique` y Arrays → `std::vector`

## Resumen Ejecutivo

**Objetivo**: Migrar de `new`/`new[]` a `std::make_unique` y `std::vector` para mejor gestión de memoria (RAII).

**Archivos afectados**: `RMesh_Load.cpp`, `RMesh_Render.cpp`

**Total de ocurrencias**: 12 usos de `new`/`new[]`

---

## 1. Análisis de Ownership

### 1.1 RMeshNode (2 ocurrencias)

**Ubicaciones**:
- Línea 788: `RMeshNode* pMeshNode = new RMeshNode;` (en `ReadElu`)
- Línea 1075: `RMeshNode* pMeshNode = new RMeshNode;` (en `AddNode`)

**Almacenamiento**:
```cpp
m_list.PushBack(pMeshNode);        // RMeshNodeHashList (contiene punteros)
m_data.push_back(pMeshNode);       // std::vector<RMeshNode*>
```

**Ownership**: ✅ **RMesh es el owner** - Los nodos pertenecen al mesh y se destruyen cuando el mesh se destruye.

**Estrategia**: Cambiar contenedores a `std::unique_ptr<RMeshNode>` o mantener raw pointers si el contenedor no soporta unique_ptr fácilmente.

---

### 1.2 m_parts_mgr (1 ocurrencia)

**Ubicación**: Línea 102: `m_parts_mgr = new RMeshMgr;`

**Tipo actual**: `RMeshMgr* m_parts_mgr;` (miembro de `RMesh`)

**Ownership**: ✅ **RMesh es el owner** - Se destruye en `RMesh::Destroy()`

**Estrategia**: Cambiar a `std::unique_ptr<RMeshMgr> m_parts_mgr;`

---

### 1.3 Arrays en RMeshNode (7 ocurrencias)

**Ubicaciones**:
- Línea 860: `pMeshNode->m_point_list = new rvector[...];`
- Línea 872: `pMeshNode->m_face_list = new RFaceInfo[...];`
- Línea 873: `pMeshNode->m_face_normal_list = new RFaceNormalInfo[...];`
- Línea 889: `RFaceInfoOld* pInfo = new RFaceInfoOld[...];` (temporal)
- Línea 903: `pMeshNode->m_point_color_list = new rvector[...];`
- Línea 922: `pMeshNode->m_physique = new RPhysiqueInfo[...];`
- Línea 954: `rvector* pPointNormal = new rvector[...];` (temporal)

**Tipos actuales** (en `RMeshNode`):
```cpp
rvector* m_point_list;
RFaceInfo* m_face_list;
RFaceNormalInfo* m_face_normal_list;
rvector* m_point_color_list;
RPhysiqueInfo* m_physique;
```

**Ownership**: ✅ **RMeshNode es el owner** - Se destruyen en `~RMeshNode()`

**Estrategia**: Cambiar a `std::vector<T>` o `std::unique_ptr<T[]>` según el caso.

---

### 1.4 RRenderNode (1 ocurrencia)

**Ubicación**: Línea 667: `RRenderNode* pNode = new RRenderNode;` (en `RMesh_Render.cpp`)

**Almacenamiento**:
```cpp
m_RenderNodeList[mode].push_back(pNode);  // std::list<RRenderNode*>
```

**Ownership**: ✅ **RRenderNodeMgr es el owner** - Se destruyen en `Clear()`

**Estrategia**: Cambiar a `std::list<std::unique_ptr<RRenderNode>>`

---

### 1.5 Arrays temporales (2 ocurrencias)

**Ubicaciones**:
- Línea 889: `RFaceInfoOld* pInfo = new RFaceInfoOld[...];` (se libera con `delete[]`)
- Línea 954: `rvector* pPointNormal = new rvector[...];` (se libera con `delete[]`)

**Ownership**: ✅ **Local scope** - Se destruyen antes de salir de la función

**Estrategia**: Usar `std::vector<T>` o `std::unique_ptr<T[]>` para RAII automático.

---

## 2. Plan de Migración por Categoría

### 2.1 ✅ **FÁCIL: Arrays Temporales**

**Prioridad**: 🔴 **ALTA** (más seguro, menos impacto)

#### Caso 1: `pInfo` (línea 889)

```cpp
// ❌ Antes:
RFaceInfoOld* pInfo = new RFaceInfoOld[pMeshNode->m_face_num];
MZF_READ(pInfo, sizeof(RFaceInfoOld)*pMeshNode->m_face_num);
ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo, pMeshNode->m_face_num);
delete[] pInfo;

// ✅ Opción A: std::vector (recomendado)
std::vector<RFaceInfoOld> pInfo(pMeshNode->m_face_num);
MZF_READ(pInfo.data(), sizeof(RFaceInfoOld)*pMeshNode->m_face_num);
ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo.data(), pMeshNode->m_face_num);
// No necesita delete[] - se destruye automáticamente

// ✅ Opción B: std::unique_ptr<T[]>
auto pInfo = std::make_unique<RFaceInfoOld[]>(pMeshNode->m_face_num);
MZF_READ(pInfo.get(), sizeof(RFaceInfoOld)*pMeshNode->m_face_num);
ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo.get(), pMeshNode->m_face_num);
// No necesita delete[] - se destruye automáticamente
```

**Recomendación**: ✅ **Opción A (std::vector)** - Más simple y legible

---

#### Caso 2: `pPointNormal` (línea 954)

```cpp
// ❌ Antes:
rvector* pPointNormal = new rvector[pMeshNode->m_point_num];
memset(pPointNormal, 0, sizeof(rvector)*pMeshNode->m_point_num);
// ... uso ...
delete[] pPointNormal;

// ✅ Opción A: std::vector (recomendado)
std::vector<rvector> pPointNormal(pMeshNode->m_point_num);
// std::vector inicializa a cero automáticamente, pero si necesitas memset:
// memset(pPointNormal.data(), 0, sizeof(rvector)*pMeshNode->m_point_num);
// ... uso ...
// No necesita delete[] - se destruye automáticamente

// ✅ Opción B: std::unique_ptr<T[]>
auto pPointNormal = std::make_unique<rvector[]>(pMeshNode->m_point_num);
memset(pPointNormal.get(), 0, sizeof(rvector)*pMeshNode->m_point_num);
// ... uso ...
// No necesita delete[] - se destruye automáticamente
```

**Recomendación**: ✅ **Opción A (std::vector)** - Más simple

---

### 2.2 🟡 **MEDIO: m_parts_mgr**

**Prioridad**: 🟡 **MEDIA** (requiere cambio en header)

#### Cambios Necesarios

**1. Header (`RMesh.h`)**:
```cpp
// ❌ Antes:
RMeshMgr* m_parts_mgr;

// ✅ Después:
std::unique_ptr<RMeshMgr> m_parts_mgr;
```

**2. Implementación (`RMesh_Load.cpp` línea 102)**:
```cpp
// ❌ Antes:
if(!m_parts_mgr) {
    m_parts_mgr = new RMeshMgr;
}

// ✅ Después:
if(!m_parts_mgr) {
    m_parts_mgr = std::make_unique<RMeshMgr>();
}
```

**3. Destructor (`RMesh.cpp`)**:
```cpp
// ❌ Antes:
if(m_parts_mgr) {
    delete m_parts_mgr;
    m_parts_mgr = NULL;
}

// ✅ Después:
// No necesita nada - unique_ptr se destruye automáticamente
// O simplemente: m_parts_mgr.reset();
```

**4. Uso (donde se accede a m_parts_mgr)**:
```cpp
// ❌ Antes:
if(m_parts_mgr) {
    m_parts_mgr->SomeMethod();
}

// ✅ Después (igual funciona):
if(m_parts_mgr) {
    m_parts_mgr->SomeMethod();  // unique_ptr tiene operator->
}
```

**Impacto**: Bajo - `unique_ptr` tiene `operator->` y `operator*`, así que el código existente funciona igual.

---

### 2.3 🟡 **MEDIO: Arrays en RMeshNode**

**Prioridad**: 🟡 **MEDIA** (requiere cambios en header y destructor)

#### Estrategia: Usar `std::vector`

**Ventajas**:
- ✅ RAII automático
- ✅ No necesita `delete[]` en destructor
- ✅ Más seguro (bounds checking opcional)
- ✅ Compatible con código existente (`.data()` para punteros)

**Desventajas**:
- ⚠️ Cambio en header (todos los archivos que incluyen `RMeshNode.h` se recompilan)
- ⚠️ Cambio en destructor (eliminar `delete[]`)

#### Cambios Necesarios

**1. Header (`RMeshNode.h` o `RMeshNodeData.h`)**:
```cpp
// ❌ Antes:
rvector* m_point_list;
RFaceInfo* m_face_list;
RFaceNormalInfo* m_face_normal_list;
rvector* m_point_color_list;
RPhysiqueInfo* m_physique;

// ✅ Después:
std::vector<rvector> m_point_list;
std::vector<RFaceInfo> m_face_list;
std::vector<RFaceNormalInfo> m_face_normal_list;
std::vector<rvector> m_point_color_list;
std::vector<RPhysiqueInfo> m_physique;
```

**2. Implementación (`RMesh_Load.cpp`)**:

**Línea 860**:
```cpp
// ❌ Antes:
pMeshNode->m_point_list = new rvector[pMeshNode->m_point_num];
memset(pMeshNode->m_point_list, 0, pMeshNode->m_point_num * sizeof(rvector));
MZF_READ(pMeshNode->m_point_list, sizeof(rvector)*pMeshNode->m_point_num);

// ✅ Después:
pMeshNode->m_point_list.resize(pMeshNode->m_point_num);
// std::vector inicializa a cero, pero si necesitas memset explícito:
// memset(pMeshNode->m_point_list.data(), 0, pMeshNode->m_point_num * sizeof(rvector));
MZF_READ(pMeshNode->m_point_list.data(), sizeof(rvector)*pMeshNode->m_point_num);
```

**Línea 872-873**:
```cpp
// ❌ Antes:
pMeshNode->m_face_list = new RFaceInfo[pMeshNode->m_face_num];
pMeshNode->m_face_normal_list = new RFaceNormalInfo[pMeshNode->m_face_num];
memset(pMeshNode->m_face_list, 0, pMeshNode->m_face_num * sizeof(RFaceInfo));

// ✅ Después:
pMeshNode->m_face_list.resize(pMeshNode->m_face_num);
pMeshNode->m_face_normal_list.resize(pMeshNode->m_face_num);
// std::vector inicializa a cero automáticamente
```

**Línea 903**:
```cpp
// ❌ Antes:
pMeshNode->m_point_color_list = new rvector[pMeshNode->m_point_color_num];
MZF_READ(pMeshNode->m_point_color_list, sizeof(rvector)*pMeshNode->m_point_color_num);

// ✅ Después:
pMeshNode->m_point_color_list.resize(pMeshNode->m_point_color_num);
MZF_READ(pMeshNode->m_point_color_list.data(), sizeof(rvector)*pMeshNode->m_point_color_num);
```

**Línea 922**:
```cpp
// ❌ Antes:
pMeshNode->m_physique = new RPhysiqueInfo[pMeshNode->m_physique_num];
ZeroMemory(pMeshNode->m_physique, pMeshNode->m_physique_num * sizeof(RPhysiqueInfo));

// ✅ Después:
pMeshNode->m_physique.resize(pMeshNode->m_physique_num);
// std::vector inicializa a cero automáticamente, pero si necesitas ZeroMemory:
// ZeroMemory(pMeshNode->m_physique.data(), pMeshNode->m_physique_num * sizeof(RPhysiqueInfo));
```

**3. Destructor (`RMeshNode.cpp` o donde esté)**:
```cpp
// ❌ Antes:
~RMeshNode() {
    if(m_point_list) delete[] m_point_list;
    if(m_face_list) delete[] m_face_list;
    if(m_face_normal_list) delete[] m_face_normal_list;
    if(m_point_color_list) delete[] m_point_color_list;
    if(m_physique) delete[] m_physique;
}

// ✅ Después:
~RMeshNode() {
    // No necesita nada - std::vector se destruye automáticamente
}
```

**4. Uso en código existente**:
```cpp
// ❌ Antes:
for(int i = 0; i < m_point_num; i++) {
    rvector& v = m_point_list[i];
    // ...
}

// ✅ Después (igual funciona):
for(int i = 0; i < m_point_num; i++) {
    rvector& v = m_point_list[i];  // std::vector tiene operator[]
    // ...
}

// O mejor aún (C++11 range-based for):
for(auto& v : m_point_list) {
    // ...
}
```

**Impacto**: Medio - Requiere cambios en header y recompilación, pero el código de uso sigue funcionando igual.

---

### 2.4 🔴 **COMPLEJO: RMeshNode en contenedores**

**Prioridad**: 🔴 **BAJA** (requiere cambios extensos)

#### Análisis

**Contenedores actuales**:
```cpp
RMeshNodeHashList m_list;              // RHashList<RMeshNode*>
std::vector<RMeshNode*> m_data;       // std::vector<RMeshNode*>
```

**Problema**: Estos contenedores almacenan raw pointers. Para usar `unique_ptr`, necesitaríamos:
- Cambiar `RMeshNodeHashList` a `RHashList<std::unique_ptr<RMeshNode>>` (si es posible)
- Cambiar `std::vector<RMeshNode*>` a `std::vector<std::unique_ptr<RMeshNode>>`

**Evaluación**:
- ⚠️ `RHashList` es un tipo personalizado - necesitaría verificar si soporta `unique_ptr`
- ⚠️ Cambios extensos en todo el código que itera sobre estos contenedores
- ⚠️ Alto riesgo de introducir bugs

**Recomendación**: ⚠️ **NO MIGRAR** por ahora, o hacerlo en una refactorización mayor.

**Alternativa**: Mantener raw pointers pero asegurar que se destruyen correctamente en `~RMesh()`:
```cpp
~RMesh() {
    for(auto* node : m_data) {
        delete node;  // Asegurar cleanup
    }
    m_data.clear();
    m_list.clear();
}
```

---

### 2.5 🟡 **MEDIO: RRenderNode**

**Prioridad**: 🟡 **MEDIA** (requiere cambio en header)

#### Cambios Necesarios

**1. Header (`RMesh.h`)**:
```cpp
// ❌ Antes:
RRenderNodeList m_RenderNodeList[eRRenderNode_End];  // std::list<RRenderNode*>

// ✅ Después:
std::list<std::unique_ptr<RRenderNode>> m_RenderNodeList[eRRenderNode_End];
```

**2. Implementación (`RMesh_Render.cpp` línea 667)**:
```cpp
// ❌ Antes:
RRenderNode* pNode = new RRenderNode;
pNode->Set(mode, m, pMNode, nMtrl, 0, 0, 1.f);
m_RenderNodeList[mode].push_back(pNode);

// ✅ Después:
auto pNode = std::make_unique<RRenderNode>();
pNode->Set(mode, m, pMNode, nMtrl, 0, 0, 1.f);
m_RenderNodeList[mode].push_back(std::move(pNode));
```

**3. Uso (donde se itera)**:
```cpp
// ❌ Antes:
for(auto* node : m_RenderNodeList[mode]) {
    node->Render();
}

// ✅ Después:
for(auto& node : m_RenderNodeList[mode]) {
    node->Render();  // unique_ptr tiene operator->
}
```

**4. Clear()**:
```cpp
// ❌ Antes:
void Clear() {
    for(auto* node : m_RenderNodeList[mode]) {
        delete node;
    }
    m_RenderNodeList[mode].clear();
}

// ✅ Después:
void Clear() {
    m_RenderNodeList[mode].clear();  // unique_ptr se destruye automáticamente
}
```

**Impacto**: Medio - Requiere cambios en header y uso, pero es manejable.

---

## 3. Plan de Implementación Recomendado

### Fase 1: Cambios de Bajo Riesgo (Inmediato)

1. ✅ **Arrays temporales** → `std::vector`
   - Línea 889: `pInfo`
   - Línea 954: `pPointNormal`
   - **Riesgo**: ✅ Muy bajo
   - **Esfuerzo**: Bajo

---

### Fase 2: Cambios de Medio Riesgo (Corto Plazo)

2. ✅ **m_parts_mgr** → `std::unique_ptr<RMeshMgr>`
   - **Riesgo**: 🟡 Bajo-Medio
   - **Esfuerzo**: Medio

3. ✅ **RRenderNode** → `std::unique_ptr<RRenderNode>`
   - **Riesgo**: 🟡 Medio
   - **Esfuerzo**: Medio

---

### Fase 3: Cambios de Alto Riesgo (Largo Plazo)

4. ⚠️ **Arrays en RMeshNode** → `std::vector<T>`
   - **Riesgo**: 🟡 Medio-Alto (requiere recompilación extensa)
   - **Esfuerzo**: Alto
   - **Recomendación**: Hacer en refactorización mayor

5. ❌ **RMeshNode en contenedores** → `std::unique_ptr<RMeshNode>`
   - **Riesgo**: 🔴 Alto (cambios extensos)
   - **Esfuerzo**: Muy Alto
   - **Recomendación**: NO hacer por ahora, o en refactorización completa

---

## 4. Ejemplos de Implementación Completos

### 4.1 Arrays Temporales (Fase 1)

**Archivo**: `RMesh_Load.cpp`

#### Caso 1: `pInfo` (línea 889)

```cpp
// ❌ Antes:
RFaceInfoOld* pInfo = new RFaceInfoOld[pMeshNode->m_face_num];
MZF_READ(pInfo, sizeof(RFaceInfoOld)*pMeshNode->m_face_num);
ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo, pMeshNode->m_face_num);
delete[] pInfo;

// ✅ Después:
std::vector<RFaceInfoOld> pInfo(pMeshNode->m_face_num);
MZF_READ(pInfo.data(), sizeof(RFaceInfoOld)*pMeshNode->m_face_num);
ConvertOldFaceInfo(pMeshNode->m_face_list, pInfo.data(), pMeshNode->m_face_num);
// delete[] eliminado - RAII automático
```

#### Caso 2: `pPointNormal` (línea 954)

```cpp
// ❌ Antes:
rvector* pPointNormal = new rvector[pMeshNode->m_point_num];
memset(pPointNormal, 0, sizeof(rvector)*pMeshNode->m_point_num);

for(k=0; k<pMeshNode->m_face_num; k++) {
    for(j=0; j<3; j++) {
        // ... uso de pPointNormal ...
    }
}

delete[] pPointNormal;

// ✅ Después:
std::vector<rvector> pPointNormal(pMeshNode->m_point_num);
// std::vector inicializa a cero, pero si necesitas memset explícito:
// memset(pPointNormal.data(), 0, sizeof(rvector)*pMeshNode->m_point_num);

for(k=0; k<pMeshNode->m_face_num; k++) {
    for(j=0; j<3; j++) {
        // ... uso de pPointNormal[j] (igual que antes) ...
    }
}
// delete[] eliminado - RAII automático
```

---

### 4.2 m_parts_mgr (Fase 2)

**Archivo**: `RMesh.h`

```cpp
// ❌ Antes:
RMeshMgr* m_parts_mgr;

// ✅ Después:
std::unique_ptr<RMeshMgr> m_parts_mgr;
```

**Archivo**: `RMesh_Load.cpp` (línea 102)

```cpp
// ❌ Antes:
if(!m_parts_mgr) {
    m_parts_mgr = new RMeshMgr;
}

// ✅ Después:
if(!m_parts_mgr) {
    m_parts_mgr = std::make_unique<RMeshMgr>();
}
```

**Archivo**: `RMesh.cpp` (destructor)

```cpp
// ❌ Antes:
void RMesh::Destroy() {
    if(m_parts_mgr) {
        delete m_parts_mgr;
        m_parts_mgr = nullptr;
    }
}

// ✅ Después:
void RMesh::Destroy() {
    m_parts_mgr.reset();  // O simplemente dejar que se destruya automáticamente
}
```

---

### 4.3 RRenderNode (Fase 2)

**Archivo**: `RMesh.h`

```cpp
// ❌ Antes:
class RRenderNodeList : public std::list<RRenderNode*>

// ✅ Después:
class RRenderNodeList : public std::list<std::unique_ptr<RRenderNode>>
```

**Archivo**: `RMesh_Render.cpp` (línea 667)

```cpp
// ❌ Antes:
RRenderNode* pNode = new RRenderNode;
pNode->Set(mode, m, pMNode, nMtrl, 0, 0, 1.f);
m_RenderNodeList[mode].push_back(pNode);

// ✅ Después:
auto pNode = std::make_unique<RRenderNode>();
pNode->Set(mode, m, pMNode, nMtrl, 0, 0, 1.f);
m_RenderNodeList[mode].push_back(std::move(pNode));
```

**Archivo**: `RMesh.h` (método Clear)

```cpp
// ❌ Antes:
void Clear() {
    for(auto* node : m_RenderNodeList[mode]) {
        delete node;
    }
    m_RenderNodeList[mode].clear();
}

// ✅ Después:
void Clear() {
    m_RenderNodeList[mode].clear();  // unique_ptr se destruye automáticamente
}
```

---

## 5. Checklist de Migración

### Fase 1: Arrays Temporales ✅

- [ ] Línea 889: `pInfo` → `std::vector<RFaceInfoOld>`
- [ ] Línea 954: `pPointNormal` → `std::vector<rvector>`
- [ ] Eliminar `delete[]` correspondientes
- [ ] Verificar que `.data()` se usa correctamente

### Fase 2: Objetos Únicos 🟡

- [ ] `m_parts_mgr` → `std::unique_ptr<RMeshMgr>`
  - [ ] Cambiar en `RMesh.h`
  - [ ] Cambiar creación en `RMesh_Load.cpp`
  - [ ] Cambiar destructor en `RMesh.cpp`
  - [ ] Verificar todos los usos
- [ ] `RRenderNode` → `std::unique_ptr<RRenderNode>`
  - [ ] Cambiar `RRenderNodeList` en `RMesh.h`
  - [ ] Cambiar creación en `RMesh_Render.cpp`
  - [ ] Cambiar `Clear()` en `RMesh.h`
  - [ ] Verificar todos los usos

### Fase 3: Arrays en RMeshNode ⚠️

- [ ] Cambiar tipos en `RMeshNode.h` o `RMeshNodeData.h`
- [ ] Cambiar todas las asignaciones en `RMesh_Load.cpp`
- [ ] Cambiar destructor de `RMeshNode`
- [ ] Verificar todos los usos (`.data()` para punteros)
- [ ] Recompilar todo el proyecto

---

## 6. Beneficios Esperados

1. ✅ **RAII automático**: No más `delete`/`delete[]` manuales
2. ✅ **Exception safety**: Si hay excepción, la memoria se libera automáticamente
3. ✅ **Menos bugs**: Imposible olvidar `delete`
4. ✅ **Código más moderno**: Alineado con C++14
5. ✅ **Mejor debugging**: `std::vector` tiene mejor soporte en debuggers

---

## 7. Riesgos y Mitigación

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|------------|
| Cambios en header causan recompilación extensa | Alta | Medio | Hacer en fases, probar cada fase |
| Bugs en migración de contenedores | Media | Alto | Hacer en fases pequeñas, testing exhaustivo |
| Performance (overhead de std::vector) | Baja | Bajo | `std::vector` es muy eficiente, overhead mínimo |
| Compatibilidad con código existente | Media | Medio | Usar `.data()` para obtener punteros cuando sea necesario |

---

## 8. Conclusión

**Recomendación**: Empezar con **Fase 1** (arrays temporales) que tiene riesgo muy bajo y beneficio inmediato. Luego evaluar **Fase 2** según resultados.

**Fase 3** (arrays en RMeshNode) requiere más planificación y testing, hacerla en una refactorización mayor.

**Fase 4** (RMeshNode en contenedores) es muy compleja, mejor dejarla para una refactorización completa del sistema de meshes.

