# Optimizaciones de Rendimiento: Sistema de Luces

## 🔍 Problema Identificado

El usuario reportó que el sistema de luces consume **muchos recursos**, específicamente:
1. **GunLight**: La luz generada al disparar armas
2. **Luces de colisión**: Luces generadas cuando skills colisionan o explotan

## ✅ Optimizaciones Aplicadas

### 1. Optimización de `SetGunLight()` (ZCharacterObject.cpp)

**Problema**: Se ejecutaba cada frame para cada personaje, incluso cuando no había luz activa.

**Solución**:
- ✅ **Early exit** si no hay luces dinámicas habilitadas o no hay luz activa
- ✅ Verificación de expiración **antes** de hacer cálculos costosos
- ✅ Solo actualizar posición y establecer luz si realmente hay luz activa

**Código**:
```cpp
void ZCharacterObject::SetGunLight()
{
    // OPTIMIZACIÓN: Early exit si no hay luces dinámicas habilitadas o no hay luz activa
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight || !m_bDynamicLight)
    {
        if (m_bDynamicLight)
        {
            m_pVMesh->SetLight(0, nullptr, false);
            m_bDynamicLight = false;
        }
        return;
    }

    // Verificar si la luz expiró antes de hacer cálculos costosos
    float currentTime = GetGlobalTimeMS();
    float lap = currentTime - m_fTime;
    m_fTime = currentTime;
    m_fLightLife -= lap;

    if (m_fLightLife <= 0.0f)
    {
        m_bDynamicLight = false;
        m_fLightLife = 0;
        m_pVMesh->SetLight(0, nullptr, false);
        return;
    }
    // ... resto del código solo se ejecuta si hay luz activa
}
```

**Impacto**: Reduce significativamente el costo cuando no hay disparos activos.

---

### 2. Límite de Luces en `ZStencilLight` (ZStencilLight.cpp)

**Problema**: Las luces se acumulaban infinitamente sin límite, causando degradación de rendimiento.

**Solución**:
- ✅ **Límite máximo**: `MAX_ACTIVE_LIGHTS = 50` luces activas simultáneas
- ✅ **Limpieza inteligente**: Elimina luces expiradas primero
- ✅ **Priorización**: Elimina luces sin atenuación antes que las temporales

**Código**:
```cpp
#define MAX_ACTIVE_LIGHTS    50  // Límite máximo de luces activas

int ZStencilLight::AddLightSource(const rvector& p, float power, DWORD lastTime)
{
    // Si hay demasiadas luces, limpiar primero
    if (m_LightSource.size() >= MAX_ACTIVE_LIGHTS)
    {
        // 1. Eliminar luces expiradas
        DWORD currentTime = GetGlobalTimeMS();
        for (auto iter = m_LightSource.begin(); iter != m_LightSource.end(); )
        {
            LightSource* pLS = iter->second;
            if (pLS && pLS->bAttenuation && currentTime >= pLS->deadTime)
            {
                SAFE_DELETE(pLS);
                iter = m_LightSource.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        
        // 2. Si aún hay demasiadas, eliminar luces sin atenuación
        if (m_LightSource.size() >= MAX_ACTIVE_LIGHTS)
        {
            // ... eliminar la más antigua sin atenuación
        }
    }
    // ... agregar nueva luz
}
```

**Impacto**: Previene acumulación infinita de luces, especialmente en combates intensos con muchos disparos.

---

### 3. Optimización de `SetMapLight()` (ZCharacterObject.cpp)

**Problema**: Iteraba sobre **todas** las luces del mapa cada frame para cada personaje, incluso las muy lejanas.

**Solución**:
- ✅ **Early exit** si no hay luces en el mapa
- ✅ **Distancia máxima**: `MAX_LIGHT_DISTANCE = 5000.0f` unidades
- ✅ **Skip luces lejanas**: No procesa luces fuera del rango útil

**Código**:
```cpp
static RLIGHT* SetMapLight(const v3& char_pos, RVisualMesh* Mesh, int LightIndex, RLIGHT* FirstLight)
{
    // OPTIMIZACIÓN: Early exit si no hay luces en el mapa
    auto& SunLightList = ZGetGame()->GetWorld()->GetBsp()->GetSunLightList();
    auto& ObjectLightList = ZGetGame()->GetWorld()->GetBsp()->GetObjectLightList();
    
    if (SunLightList.empty() && ObjectLightList.empty())
    {
        Mesh->SetLight(LightIndex, nullptr, false);
        return nullptr;
    }

    const float MAX_LIGHT_DISTANCE = 5000.0f; // Distancia máxima

    // ... buscar luces, pero skip las muy lejanas
    for (auto& Light : SunLightList)
    {
        distance = MagnitudeSq(sunDir);
        
        // OPTIMIZACIÓN: Skip luces muy lejanas
        if (distance > MAX_LIGHT_DISTANCE * MAX_LIGHT_DISTANCE)
            continue;
        // ...
    }
}
```

**Impacto**: Reduce iteraciones innecesarias, especialmente en mapas con muchas luces.

---

### 4. Optimización de `Draw_SetLight()` (ZCharacterObject.cpp)

**Problema**: Siempre buscaba luces del mapa incluso cuando no había luces dinámicas.

**Solución**:
- ✅ **Early exit completo** si no hay luces dinámicas habilitadas
- ✅ Solo busca luces del mapa si hay luces disponibles

**Código**:
```cpp
void ZCharacterObject::Draw_SetLight(const rvector& vPosition)
{
    // OPTIMIZACIÓN: Early exit si no hay luces dinámicas habilitadas
    if (!ZGetConfiguration()->GetVideo()->bDynamicLight)
    {
        m_pVMesh->SetLight(0, nullptr, false);
        m_pVMesh->SetLight(1, nullptr, false);
        m_pVMesh->SetLight(2, nullptr, false);
        RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
        return;
    }

    SetGunLight();
    // Solo buscar luces del mapa si hay luces disponibles
    // ...
}
```

**Impacto**: Evita cálculos innecesarios cuando las luces dinámicas están deshabilitadas.

---

## 📊 Resumen de Mejoras

| Optimización | Archivo | Impacto |
|-------------|---------|---------|
| **SetGunLight() Early Exit** | `ZCharacterObject.cpp` | ⭐⭐⭐⭐⭐ Reduce cálculos cuando no hay disparos |
| **Límite de Luces StencilLight** | `ZStencilLight.cpp` | ⭐⭐⭐⭐⭐ Previene acumulación infinita |
| **SetMapLight() Distancia Máxima** | `ZCharacterObject.cpp` | ⭐⭐⭐⭐ Reduce iteraciones sobre luces lejanas |
| **Draw_SetLight() Early Exit** | `ZCharacterObject.cpp` | ⭐⭐⭐⭐ Evita cálculos cuando luces deshabilitadas |

---

## 🎯 Resultados Esperados

1. **GunLight**: 
   - ✅ Solo se calcula cuando hay luz activa
   - ✅ Early exit cuando no hay disparos
   - ✅ Verificación de expiración antes de cálculos costosos

2. **Luces de Colisión (StencilLight)**:
   - ✅ Máximo 50 luces activas simultáneas
   - ✅ Limpieza automática de luces expiradas
   - ✅ Prevención de acumulación infinita

3. **Luces del Mapa**:
   - ✅ Solo procesa luces dentro de 5000 unidades
   - ✅ Early exit si no hay luces disponibles
   - ✅ Reduce iteraciones innecesarias

---

## 🔧 Configuración

### Ajustar Límite de Luces

Si necesitas cambiar el límite máximo de luces, edita en `ZStencilLight.cpp`:

```cpp
#define MAX_ACTIVE_LIGHTS    50  // Cambiar este valor
```

**Recomendaciones**:
- **Bajo rendimiento**: 30-40 luces
- **Rendimiento medio**: 50 luces (actual)
- **Alto rendimiento**: 70-100 luces

### Ajustar Distancia Máxima de Luces del Mapa

Si necesitas cambiar la distancia máxima, edita en `ZCharacterObject.cpp`:

```cpp
const float MAX_LIGHT_DISTANCE = 5000.0f; // Cambiar este valor
```

**Recomendaciones**:
- **Mapas pequeños**: 3000.0f
- **Mapas medianos**: 5000.0f (actual)
- **Mapas grandes**: 8000.0f

---

## ⚠️ Notas Importantes

1. **Compatibilidad**: Las optimizaciones son compatibles con el código existente
2. **Configuración**: Respeta la configuración de `bDynamicLight` del usuario
3. **Limpieza**: Las luces expiradas se limpian automáticamente en `Update()`
4. **Rendimiento**: Las optimizaciones son especialmente efectivas en combates intensos

---

## 🐛 Debugging

Para verificar el número de luces activas, puedes usar:

```cpp
size_t lightCount = ZGetStencilLight()->GetCount();
```

Esto te ayudará a identificar si hay acumulación excesiva de luces.

---

## 📝 Próximas Mejoras Posibles

1. **Pool de Luces**: Reutilizar objetos `LightSource` en lugar de crear/eliminar
2. **Culling Espacial**: Usar octree o grid para luces del mapa
3. **LOD de Luces**: Reducir calidad de luces lejanas
4. **Batch de Luces**: Agrupar luces cercanas para reducir draw calls

---

## ✅ Validación

Todas las optimizaciones han sido:
- ✅ Compiladas sin errores
- ✅ Probadas con linter
- ✅ Mantienen compatibilidad con código existente
- ✅ Respetan configuración del usuario

