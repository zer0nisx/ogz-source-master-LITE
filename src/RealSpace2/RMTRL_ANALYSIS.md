# Análisis Completo de RMtrl

## Resumen Ejecutivo

`RMtrl` es la clase principal que representa un material en el sistema de renderizado RealSpace2. Gestiona texturas, propiedades de iluminación, transparencia, y efectos especiales para objetos 3D. `RMtrlMgr` es el gestor que administra múltiples materiales.

---

## 1. Estructura de la Clase RMtrl

### 1.1 Definición y Ubicación
- **Archivo Header**: `RealSpace2/Include/RMtrl.h`
- **Archivo Implementación**: `RealSpace2/Source/RMtrl.cpp`
- **Namespace**: `REALSPACE2`

### 1.2 Características Principales
- **Constructor**: Inicializa todos los valores por defecto
- **Destructor**: Limpia texturas (normales y animadas)
- **Copy Constructor**: Eliminado (`= delete`) - no se permite copia
- **Gestión de Memoria**: Maneja texturas propias y las destruye en el destructor

---

## 2. Propiedades y Miembros

### 2.1 Texturas

#### Texturas Principales
```cpp
RBaseTexture*  m_pTexture;           // Textura principal del material
RBaseTexture*  m_pToonTexture;        // Textura para efecto toon/cel-shading
RBaseTexture** m_pAniTexture;         // Array de texturas para animación
```

#### Configuración de Texturas
```cpp
D3DTEXTUREFILTERTYPE m_FilterType;        // Filtro de textura principal (LINEAR por defecto)
D3DTEXTUREFILTERTYPE m_ToonFilterType;    // Filtro de textura toon (POINT por defecto)
D3DTEXTUREOP m_TextureBlendMode;          // Modo de mezcla de textura (BLENDTEXTUREALPHA)
D3DTEXTUREOP m_ToonTextureBlendMode;      // Modo de mezcla toon (MODULATE2X)
```

### 2.2 Propiedades de Iluminación (Material)

```cpp
color_r32 m_ambient;    // Color ambiente (RGBA)
color_r32 m_diffuse;    // Color difuso (RGBA)
color_r32 m_specular;   // Color especular (RGBA)
float     m_power;      // Potencia especular (shininess)
```

**Valores por defecto**:
- `m_ambient`, `m_diffuse`, `m_specular`: `0xffffffff` (blanco)
- `m_power`: `1.0f`

### 2.3 Identificadores

```cpp
int m_id;              // ID único del material
int m_u_id;            // ID de usuario (opcional, -1 por defecto)
int m_mtrl_id;         // ID del material en el mesh
int m_sub_mtrl_id;     // ID del sub-material
int m_sub_mtrl_num;    // Número de sub-materiales
```

### 2.4 Nombres de Archivos

```cpp
char m_mtrl_name[255];      // Nombre completo del material (ej: "texture0")
char m_name[255];           // Nombre del archivo de textura
char m_opa_name[255];      // Nombre del archivo de opacidad
char m_name_ani_tex[255];   // Nombre base para texturas animadas
char m_name_ani_tex_ext[20]; // Extensión de texturas animadas
```

### 2.5 Flags de Renderizado

```cpp
bool m_bDiffuseMap;        // Usa mapa de difusión
bool m_bTwoSided;          // Renderizado de dos caras (no culling)
bool m_bAlphaMap;          // Usa mapa de alpha
bool m_bAlphaTestMap;      // Usa alpha test
bool m_bAdditive;          // Modo aditivo (para efectos)
bool m_bUse;               // Material en uso (para limpieza)
bool m_bObjectMtrl;        // Material para objetos/efectos/interfaz
bool m_bAniTex;            // Textura animada activa
```

### 2.6 Configuración de Alpha

```cpp
u32  m_AlphaRefValue;      // Valor de referencia para alpha test (0x05 por defecto)
int  m_nAlphaTestValue;    // Valor de alpha test (-1 por defecto)
```

### 2.7 Texturas Animadas

```cpp
bool m_bAniTex;            // Indica si usa texturas animadas
int  m_nAniTexCnt;         // Número de texturas en la animación
int  m_nAniTexSpeed;       // Velocidad de animación (ms)
int  m_nAniTexGap;         // Intervalo entre frames (calculado)
u64  m_backup_time;        // Tiempo de respaldo para animación
```

**Formato de nombre de textura animada**: `txa_<count>_<speed>_<basename>`
- Ejemplo: `txa_10_100_test01.tga` → 10 frames, 100ms de velocidad, base "test"

### 2.8 Color TFactor

```cpp
u32 m_dwTFactorColor;      // Color para Texture Factor (D3DCOLOR)
```

**Valor por defecto**: `D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 0.0f)` (verde, alpha 0)
- Se usa para efectos de colorización en el renderizado
- Accesible mediante `GetTColor()` y `SetTColor()`

---

## 3. Métodos Públicos

### 3.1 Gestión de Texturas

#### `LPDIRECT3DTEXTURE9 GetTexture()`
- **Retorna**: La textura Direct3D9 actual
- **Lógica**:
  - Si `m_bAniTex` es true: calcula el frame actual basado en el tiempo y retorna la textura correspondiente
  - Si no: retorna `m_pTexture->GetTexture()`
- **Animación**: Usa `GetGlobalTimeMS()` para calcular el frame basado en `m_nAniTexSpeed` y `m_nAniTexGap`

#### `void CheckAniTexture()`
- **Propósito**: Detecta y configura texturas animadas desde el nombre del archivo
- **Formato detectado**: `txa_<count>_<speed>_<basename>`
- **Acciones**:
  - Parsea el nombre del archivo
  - Crea el array `m_pAniTexture`
  - Configura `m_bAniTex`, `m_nAniTexCnt`, `m_nAniTexSpeed`, `m_nAniTexGap`

#### `void Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL)`
- **Propósito**: Carga/restaura las texturas del material
- **Parámetros**:
  - `dev`: Dispositivo Direct3D9
  - `path`: Ruta opcional para buscar texturas
- **Lógica**:
  - Determina el nivel de textura (`RTextureType::Etc` o `RTextureType::Object`)
  - Si es textura animada: carga todas las texturas del array
  - Si no: carga la textura principal
  - Intenta primero con `path + m_name`, luego con `m_name` solo

### 3.2 Gestión de Color TFactor

#### `void SetTColor(u32 color)`
- **Propósito**: Establece el color TFactor
- **Uso**: Para efectos de colorización en renderizado

#### `u32 GetTColor()`
- **Propósito**: Obtiene el color TFactor actual
- **Retorna**: `m_dwTFactorColor`

---

## 4. Clase RMtrlMgr

### 4.1 Definición
```cpp
class RMtrlMgr final : public std::list<RMtrl*>
```
- **Herencia**: Hereda de `std::list<RMtrl*>` para gestión de lista
- **Final**: No puede ser heredada

### 4.2 Miembros

```cpp
std::vector<RMtrl*> m_node_table;  // Tabla de nodos (acceso rápido)
bool m_bObjectMtrl{};              // Flag para materiales de objeto
int  m_id_last{};                  // Último ID asignado
```

### 4.3 Métodos Principales

#### Gestión de Materiales

**`int Add(char* name, int u_id = -1)`**
- Crea un nuevo `RMtrl` con el nombre especificado
- Asigna ID automático (`m_id_last`)
- Retorna el ID asignado

**`int Add(RMtrl* tex)`**
- Agrega un material existente al gestor
- Asigna ID automático
- Retorna el ID asignado

**`void Del(int id)`**
- Elimina un material por ID
- Destruye la textura asociada

**`void Del(RMtrl* tex)`**
- Elimina un material por puntero
- Destruye la textura asociada

**`void DelAll()`**
- Elimina todos los materiales
- Limpia `m_node_table`
- Resetea `m_id_last` a 0

#### Búsqueda de Materiales

**`RMtrl* Get_s(int mtrl_id, int sub_id)`**
- Busca material por `mtrl_id` y `sub_id`
- Retorna `NULL` si no se encuentra

**`RMtrl* GetMtrl(char* name)`**
- Busca material por nombre (`m_name`)
- Comparación case-sensitive con `strcmp`

**`RMtrl* GetToolMtrl(char* name)`**
- Busca material por nombre completo (`m_mtrl_name`)
- Usado en herramientas de edición

#### Obtención de Texturas

**`LPDIRECT3DTEXTURE9 Get(int id)`**
- Obtiene textura por ID del material

**`LPDIRECT3DTEXTURE9 Get(int id, int sub_id)`**
- Obtiene textura por `mtrl_id` y `sub_id`

**`LPDIRECT3DTEXTURE9 GetUser(int u_id)`**
- Obtiene textura por ID de usuario

**`LPDIRECT3DTEXTURE9 Get(char* name)`**
- Obtiene textura por nombre del material

#### Gestión de Estado

**`void Restore(LPDIRECT3DDEVICE9 dev, char* path = NULL)`**
- Restaura todas las texturas de todos los materiales
- Útil después de pérdida de dispositivo

**`void ClearUsedCheck()`**
- Marca todos los materiales como no usados (`m_bUse = false`)

**`void ClearUsedMtrl()`**
- Elimina materiales no marcados como usados
- Útil para limpieza de memoria

**`int GetNum()`**
- Retorna el número de materiales en el gestor

**`void SetObjectTexture(bool bObject)`**
- Establece el flag `m_bObjectMtrl` para nuevos materiales

---

## 5. Uso en el Sistema de Renderizado

### 5.1 Integración con RMesh

`RMtrl` se usa extensivamente en `RMesh` para renderizado:

- **`SetCharacterMtrl_ON()`**: Configura el material para renderizado de personajes
- **`SetCharacterMtrl_OFF()`**: Restaura estados después del renderizado
- **`SetShaderDiffuseMap()`**: Configura shader para mapa difuso
- **`GetCharacterMtrlMode()`**: Determina el modo de renderizado según el material

### 5.2 Integración con RShaderMgr

`RShaderMgr` usa `RMtrl` para pasar propiedades de material a shaders:

```cpp
void RShaderMgr::setMtrl(RMtrl* pmtrl_, float fVisAlpha_)
```
- Copia propiedades de material (`diffuse`, `ambient`, `specular`, `power`)
- Ajusta valores para vertex shader
- Se envía a constantes de shader en `Update()`

### 5.3 Integración con RMeshNode

`RMeshNode` usa materiales para:
- Renderizado de nodos de mesh
- Efectos toon con `m_pToonTexture`
- Colorización con `GetTColor()`
- Configuración de renderizado según flags del material

### 5.4 Casos de Uso Específicos

#### Renderizado de Personajes
- `SetCharacterMtrl_ON()` configura estados D3D según flags del material
- Soporta: difuso, alpha, alpha test, aditivo, two-sided

#### Efectos Toon
- `m_pToonTexture` se usa en stage 1 del pipeline de texturas
- Configurado en `ToonRenderSettingOn()` y `ToonRenderSettingOnOld()`

#### Texturas Animadas
- `GetTexture()` calcula el frame actual basado en tiempo
- Formato: `txa_<count>_<speed>_<basename>`

#### Colorización
- `m_dwTFactorColor` se usa para efectos de color
- Accesible mediante `GetTColor()` / `SetTColor()`

---

## 6. Flujo de Trabajo Típico

### 6.1 Carga de Materiales

1. **Creación**: `RMtrlMgr::Add(name)` o `RMtrlMgr::Add(RMtrl*)`
2. **Configuración**: Establecer propiedades (flags, colores, etc.)
3. **Carga de Texturas**: `RMtrl::Restore(dev, path)`
4. **Detección de Animación**: `RMtrl::CheckAniTexture()` (automático o manual)

### 6.2 Uso en Renderizado

1. **Obtención**: `RMtrlMgr::Get_s(mtrl_id, sub_id)` o `GetMtrl(name)`
2. **Configuración de Shader**: `RShaderMgr::setMtrl(pMtrl, alpha)`
3. **Configuración de Estados**: `RMesh::SetCharacterMtrl_ON(pMtrl, ...)`
4. **Renderizado**: Usar `GetTexture()` para obtener textura actual
5. **Limpieza**: `RMesh::SetCharacterMtrl_OFF(pMtrl, alpha)`

### 6.3 Gestión de Memoria

1. **Marcado**: `RMtrlMgr::ClearUsedCheck()` marca todos como no usados
2. **Uso**: Durante renderizado, marcar materiales usados (`m_bUse = true`)
3. **Limpieza**: `RMtrlMgr::ClearUsedMtrl()` elimina no usados

---

## 7. Características Especiales

### 7.1 Texturas Animadas

- **Formato**: `txa_<count>_<speed>_<basename>`
- **Ejemplo**: `txa_10_100_test01.tga` → 10 frames, 100ms velocidad
- **Cálculo de Frame**: `pos = (time % speed) / gap`
- **Carga**: Todas las texturas se cargan en `Restore()`

### 7.2 Efectos Toon

- `m_pToonTexture`: Textura adicional para cel-shading
- `m_ToonFilterType`: Filtro POINT por defecto (pixelado)
- `m_ToonTextureBlendMode`: MODULATE2X por defecto

### 7.3 Modos de Renderizado

- **DiffuseMap**: Textura difusa con colorización
- **AlphaMap**: Transparencia con alpha blending
- **AlphaTestMap**: Alpha test con referencia configurable
- **Additive**: Modo aditivo para efectos
- **TwoSided**: Renderizado sin culling

### 7.4 Color TFactor

- Usado para efectos de colorización
- Valor por defecto: verde con alpha 0 (inactivo)
- Se puede establecer por material o por nodo de mesh

---

## 8. Dependencias

### 8.1 Incluye
- `RBaseTexture.h`: Gestión de texturas base
- `RTypes.h`: Tipos base (color_r32, u32, etc.)
- `<d3d9.h>`: Direct3D9

### 8.2 Usado Por
- `RMesh`: Renderizado de mallas
- `RMeshNode`: Nodos de malla
- `RShaderMgr`: Gestión de shaders
- `RCharCloth`: Ropa de personajes
- `ZWater`: Efectos de agua
- Herramientas: `Mcv`, `MaterialTest`

---

## 9. Limitaciones y Consideraciones

### 9.1 Limitaciones
- **Tamaño de nombres**: Limitado a 255 caracteres (arrays fijos)
- **Sin copia**: Constructor de copia eliminado (gestión manual de memoria)
- **Búsqueda lineal**: `Get_s()`, `GetMtrl()` usan búsqueda lineal O(n)
- **Sin thread-safety**: No es thread-safe

### 9.2 Mejoras Potenciales
- Usar `std::unordered_map` para búsquedas O(1)
- Agregar thread-safety si se usa en múltiples threads
- Usar `std::string` en lugar de arrays fijos
- Implementar `LoadList()` / `SaveList()` (actualmente retornan 1 sin hacer nada)

---

## 10. Ejemplos de Uso

### 10.1 Crear y Configurar Material

```cpp
RMtrlMgr mtrlMgr;
int id = mtrlMgr.Add("texture01.tga");
RMtrl* pMtrl = mtrlMgr.Get_s(id, -1);

// Configurar propiedades
pMtrl->m_bDiffuseMap = true;
pMtrl->m_bTwoSided = false;
pMtrl->m_diffuse = color_r32{1.0f, 1.0f, 1.0f, 1.0f};
pMtrl->m_power = 32.0f;

// Cargar textura
pMtrl->Restore(RGetDevice(), "textures/");
```

### 10.2 Usar Material en Renderizado

```cpp
RMtrl* pMtrl = mtrlMgr.Get_s(mtrl_id, sub_id);
if (pMtrl) {
    // Configurar shader
    RGetShaderMgr()->setMtrl(pMtrl, 1.0f);
    
    // Configurar estados
    pMesh->SetCharacterMtrl_ON(pMtrl, pMeshNode, 1.0f, 0xffffffff);
    
    // Renderizar...
    
    // Limpiar
    pMesh->SetCharacterMtrl_OFF(pMtrl, 1.0f);
}
```

### 10.3 Textura Animada

```cpp
// Nombre: "txa_10_100_test01.tga"
RMtrl* pMtrl = mtrlMgr.Add("txa_10_100_test01.tga");
pMtrl->CheckAniTexture();  // Detecta y configura animación
pMtrl->Restore(RGetDevice(), "textures/");

// En renderizado, GetTexture() retorna el frame actual automáticamente
LPDIRECT3DTEXTURE9 tex = pMtrl->GetTexture();
```

---

## 11. Referencias en el Código

### Archivos Principales
- `RealSpace2/Include/RMtrl.h`: Definición de clase
- `RealSpace2/Source/RMtrl.cpp`: Implementación
- `RealSpace2/Source/RMesh_Mtrl.cpp`: Integración con RMesh
- `RealSpace2/Source/RShaderMgr.cpp`: Integración con shaders

### Archivos que Usan RMtrl
- `RealSpace2/Source/RMeshNode.cpp`: Renderizado de nodos
- `RealSpace2/Source/RMesh_Load.cpp`: Carga de mallas
- `RealSpace2/Source/RCharCloth.cpp`: Ropa de personajes
- `Gunz/ZWater.cpp`: Efectos de agua
- `Gunz/ZClothEmblem.cpp`: Emblemas en ropa
- `tools/Mcv/`: Herramientas de edición

---

## 12. Conclusión

`RMtrl` es un componente central del sistema de renderizado que proporciona:
- ✅ Gestión completa de texturas (normales, animadas, toon)
- ✅ Propiedades de material (iluminación, transparencia)
- ✅ Múltiples modos de renderizado
- ✅ Integración con shaders y pipeline D3D9
- ✅ Gestión eficiente mediante `RMtrlMgr`

El sistema está bien estructurado pero tiene oportunidades de mejora en rendimiento (búsquedas) y funcionalidad (serialización).

