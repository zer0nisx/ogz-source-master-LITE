# ¿Qué es ZStencilLight y para qué se usa?

## 📖 Resumen

`ZStencilLight` es un **sistema de iluminación dinámica** que utiliza la técnica de **stencil buffer** para crear efectos de luz realistas en tiempo real. Se usa principalmente para iluminar el entorno cuando ocurren eventos como disparos, explosiones y colisiones de proyectiles.

---

## 🎯 Propósito Principal

`ZStencilLight` crea **luces dinámicas temporales** que iluminan el mapa cuando:
1. **Se disparan armas** (rifles, shotguns, cannons)
2. **Los proyectiles colisionan o explotan**
3. **Los skills generan efectos de luz**
4. **Las armas especiales se activan**

---

## 🔧 Cómo Funciona

### 1. Técnica de Stencil Buffer

El sistema usa **stencil buffer** para renderizar luces de forma eficiente:
- Crea una máscara en el stencil buffer
- Renderiza la luz solo en las áreas visibles
- Aplica atenuación y decaimiento temporal

### 2. Flujo de Trabajo

```
1. AddLightSource() → Agrega una nueva luz en una posición
2. Update() → Actualiza y elimina luces expiradas
3. Render() → Renderiza todas las luces activas usando stencil buffer
```

---

## 📍 Dónde se Usa

### 1. Disparos de Armas (ZGame.cpp)

#### Rifle/Pistola
```cpp
// ZGame.cpp línea 2986
void ZGame::OnPeerShot_Range(...)
{
    if (Z_VIDEO_DYNAMICLIGHT)
    {
        if (CharOwner)
        {
            CharOwner->SetLight(CharacterLight::Gun);
        }
        ZGetStencilLight()->AddLightSource(pos, 2.0f, 75);  // Luz por 75ms
    }
}
```

#### Shotgun
```cpp
// ZGame.cpp línea 3126
void ZGame::OnPeerShot_Shotgun(...)
{
    ZGetStencilLight()->AddLightSource(pos, 2.0f, 200);  // Luz por 200ms
}
```

#### Cannon
```cpp
// ZGame.cpp línea 2129
void ZGame::OnPeerShotSp(...)
{
    if (pOwnerCharacter->IsHero())
    {
        RGetDynamicLightManager()->AddLight(GUNFIRE, pos);
    }
}
```

### 2. Proyectiles y Armas (ZWeapon.cpp)

#### Cohetes (Rocket Launcher)
```cpp
// ZWeapon.cpp línea 118
void ZWeaponRocket::Create(...)
{
    m_SLSid = ZGetStencilLight()->AddLightSource(m_Position, 2.0f);
    // La luz sigue al cohete mientras vuela
}

// ZWeapon.cpp línea 196
bool ZWeaponRocket::Update(...)
{
    if (bHit)
    {
        // Al colisionar, elimina la luz del proyectil y crea una explosión
        ZGetStencilLight()->DeleteLightSource(m_SLSid);
        ZGetStencilLight()->AddLightSource(pickpos, 3.0f, 1300);  // Explosión
    }
    else
    {
        // Actualiza posición de la luz mientras vuela
        ZGetStencilLight()->SetLightSourcePosition(m_SLSid, m_Position);
    }
}
```

#### Magic/Proyectiles Mágicos
```cpp
// ZWeapon.cpp línea 944
void ZWeaponMagic::Create(...)
{
    m_SLSid = ZGetStencilLight()->AddLightSource(m_Position, 2.0f);
}
```

### 3. Skills y Efectos Especiales

```cpp
// ZGame.cpp línea 2080
case ZC_WEAPON_SP_FLASHBANG:
    ZGetStencilLight()->AddLightSource(pos, 2.0f, 100);
    break;
```

---

## 🎨 Características

### 1. Tipos de Luces

#### Luz Permanente (sin atenuación)
```cpp
AddLightSource(pos, power);  // Se mantiene hasta eliminarla manualmente
```

#### Luz Temporal (con atenuación)
```cpp
AddLightSource(pos, power, duration);  // Se desvanece después de `duration` ms
```

### 2. Parámetros

- **`pos`**: Posición 3D de la luz (rvector)
- **`power`**: Intensidad de la luz (0.0 - 1.0)
  - `2.0f` = Luz moderada (disparos normales)
  - `3.0f` = Luz intensa (explosiones)
- **`duration`**: Duración en milisegundos (solo para luces temporales)

### 3. Propiedades de Renderizado

```cpp
// ZStencilLight.cpp línea 236-260
D3DLIGHT9 light;
light.Type = D3DLIGHT_POINT;
light.Range = 300.f;  // Rango de 300 unidades
light.Diffuse.r = fPower;      // Rojo
light.Diffuse.g = 0.5 * fPower; // Verde (mitad)
light.Diffuse.b = 0.25 * fPower;// Azul (cuarto)
```

**Color**: Las luces tienen un tinte **naranja/amarillo** (más rojo, menos azul), simulando fuego/explosiones.

---

## 🔄 Ciclo de Vida

### 1. Creación
```cpp
int lightId = ZGetStencilLight()->AddLightSource(position, 2.0f, 100);
```

### 2. Actualización (cada frame)
```cpp
// ZGame.cpp línea 720
ZGetStencilLight()->Update();  // Elimina luces expiradas
```

### 3. Renderizado (cada frame)
```cpp
// ZGameDrawD3D9.cpp línea 177
ZGetStencilLight()->Render();  // Renderiza todas las luces activas
```

### 4. Eliminación
```cpp
// Automática: cuando expira (luces temporales)
// Manual: ZGetStencilLight()->DeleteLightSource(lightId);
```

---

## 💡 Ejemplos de Uso

### Ejemplo 1: Luz de Disparo
```cpp
// Cuando un personaje dispara
rvector muzzlePos;
pCharacter->GetWeaponTypePos(weapon_dummy_muzzle_flash, &muzzlePos);
ZGetStencilLight()->AddLightSource(muzzlePos, 2.0f, 75);  // Flash de 75ms
```

### Ejemplo 2: Explosión de Cohete
```cpp
// Cuando un cohete impacta
ZGetStencilLight()->AddLightSource(impactPos, 3.0f, 1300);  // Explosión brillante por 1.3s
```

### Ejemplo 3: Luz que Sigue un Proyectil
```cpp
// Crear luz al lanzar
int lightId = ZGetStencilLight()->AddLightSource(projectilePos, 2.0f);

// Actualizar posición cada frame
ZGetStencilLight()->SetLightSourcePosition(lightId, projectilePos);

// Eliminar al impactar
ZGetStencilLight()->DeleteLightSource(lightId);
ZGetStencilLight()->AddLightSource(impactPos, 3.0f, 500);  // Flash de impacto
```

---

## ⚙️ Optimizaciones Aplicadas

### 1. Límite de Luces Activas
- **Máximo**: 50 luces simultáneas (`MAX_ACTIVE_LIGHTS`)
- **Limpieza automática**: Elimina luces expiradas primero
- **Priorización**: Elimina luces sin atenuación antes que temporales

### 2. Culling de Visibilidad
```cpp
// ZStencilLight.cpp línea 234
if(!isInViewFrustum(pLS->pos, RGetViewFrustum())) continue;
```
Solo renderiza luces que están dentro del frustum de la cámara.

### 3. Actualización Eficiente
- Solo actualiza luces activas
- Elimina automáticamente luces expiradas
- No renderiza si no hay luces

---

## 🎮 Impacto Visual

### Sin ZStencilLight
- Disparos sin iluminación
- Explosiones sin efecto de luz
- Ambiente estático

### Con ZStencilLight
- ✅ **Flash de disparo**: Ilumina brevemente el área alrededor del cañón
- ✅ **Explosiones brillantes**: Iluminan el área de impacto
- ✅ **Proyectiles luminosos**: Dejan rastro de luz mientras vuelan
- ✅ **Ambiente dinámico**: El mapa se ilumina según los eventos

---

## 🔍 Diferencias con Otros Sistemas de Luz

### ZStencilLight vs GunLight (SetGunLight)
- **ZStencilLight**: Ilumina el **mapa/entorno** (BSP)
- **GunLight**: Ilumina el **personaje** (mesh del personaje)

### ZStencilLight vs RGetDynamicLightManager
- **ZStencilLight**: Usa stencil buffer, más eficiente para muchas luces pequeñas
- **DynamicLightManager**: Sistema más complejo, para luces más grandes/persistentes

---

## 📊 Estadísticas

Para ver cuántas luces están activas:
```cpp
size_t activeLights = ZGetStencilLight()->GetCount();
```

Esto es útil para debugging y optimización.

---

## ⚠️ Notas Importantes

1. **Rendimiento**: Cada luz requiere renderizado adicional
   - Por eso se limitó a 50 luces máximas
   - Las luces temporales se eliminan automáticamente

2. **Configuración**: Respeta `Z_VIDEO_DYNAMICLIGHT`
   - Si está deshabilitado, no se crean luces
   - Ahorra rendimiento en hardware débil

3. **Limpieza**: Las luces expiradas se eliminan en `Update()`
   - No hay memory leaks
   - El sistema se auto-limpia

---

## 🎯 Resumen

**ZStencilLight** es el sistema que crea **efectos de iluminación dinámica** cuando:
- Se disparan armas
- Los proyectiles explotan
- Los skills generan luz
- Ocurren eventos visuales importantes

**Técnica**: Usa stencil buffer para renderizar luces de forma eficiente.

**Optimización**: Límite de 50 luces, culling de visibilidad, limpieza automática.

**Resultado**: Ambiente dinámico e inmersivo con iluminación realista de eventos.

