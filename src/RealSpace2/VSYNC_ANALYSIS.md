# Análisis de Técnicas Anti-Tearing en RealSpace2

## 🔍 Resumen

RealSpace2 implementa **VSync** como técnica principal para evitar el screen tearing (deepframe). Sin embargo, **está desactivado por defecto**.

---

## ✅ Técnicas Implementadas

### 1. **VSync (Vertical Synchronization)** - ✅ IMPLEMENTADO

**Ubicación**: `src/RealSpace2/Source/RealSpace2.cpp`

**Función**:
```cpp
void SetVSync(bool b)
{
    if (b)
    {
        g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;  // VSync ON
        g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    }
    else
    {
        g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  // VSync OFF
        g_d3dpp.FullScreen_RefreshRateInHz = 0;
    }
}
```

**Estado por Defecto**: ❌ **DESACTIVADO**
- En `InitD3D9()` (línea 363): `g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;`
- Esto significa que **VSync está desactivado por defecto**

**Cómo Funciona**:
- `D3DPRESENT_INTERVAL_ONE`: Sincroniza el frame con el refresh rate del monitor (ej: 60Hz = 60 FPS máximo)
- `D3DPRESENT_INTERVAL_IMMEDIATE`: Presenta frames inmediatamente sin esperar al refresh rate (puede causar tearing)

---

### 2. **Swap Effect** - ✅ IMPLEMENTADO

**Ubicación**: `src/RealSpace2/Source/RealSpace2.cpp` (línea 335)

**Configuración**:
```cpp
g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
```

**Tipo**: `D3DSWAPEFFECT_DISCARD`
- **Ventaja**: Más eficiente (permite al driver optimizar)
- **Desventaja**: Puede causar tearing si VSync está desactivado
- **Alternativas disponibles**:
  - `D3DSWAPEFFECT_FLIP`: Mejor para evitar tearing, pero requiere más memoria
  - `D3DSWAPEFFECT_COPY`: Más seguro pero menos eficiente

---

### 3. **Back Buffer Count** - ✅ IMPLEMENTADO

**Ubicación**: `src/RealSpace2/Source/RealSpace2.cpp` (línea 61, 338)

**Configuración**:
```cpp
constexpr bool bTripleBuffer = false;  // Triple buffering desactivado
g_d3dpp.BackBufferCount = bTripleBuffer ? 2 : 1;  // 1 back buffer = doble buffering
```

**Tipo**: **Doble Buffering** (1 back buffer)
- **Ventaja**: Menor uso de memoria
- **Desventaja**: Puede causar stuttering si VSync está activo y el FPS cae
- **Triple Buffering**: Desactivado (requeriría `BackBufferCount = 2`)

---

### 4. **Presentación** - ✅ IMPLEMENTADO

**Ubicación**: `src/RealSpace2/Source/RealSpace2.cpp` (línea 599)

**Función**:
```cpp
void RFlip()
{
    RFrame_PrePresent();
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}
```

**Tipo**: `Present()` estándar de DirectX 9
- Respeta el `PresentationInterval` configurado
- Si VSync está activo, espera al refresh rate
- Si VSync está desactivado, presenta inmediatamente

---

## ⚠️ Problema Identificado

### VSync Desactivado por Defecto

**Línea 363 de `RealSpace2.cpp`**:
```cpp
g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  // VSync OFF
```

**Consecuencias**:
- ❌ **Screen tearing** puede ocurrir cuando el FPS es mayor que el refresh rate
- ❌ **Frames desincronizados** con el monitor
- ✅ **Mayor FPS** (sin límite de refresh rate)
- ✅ **Menor input lag** (frames se presentan inmediatamente)

---

## 🔧 Soluciones Disponibles

### Opción 1: Activar VSync Manualmente

**Llamar `SetVSync(true)` después de inicializar el dispositivo**:
```cpp
// Después de InitD3D9() o RResetDevice()
SetVSync(true);  // Activa VSync
```

**Ventajas**:
- ✅ Elimina screen tearing
- ✅ Frames sincronizados con el monitor
- ✅ Experiencia visual más suave

**Desventajas**:
- ⚠️ Limita FPS al refresh rate (ej: 60 FPS en monitor 60Hz)
- ⚠️ Puede causar stuttering si el FPS cae por debajo del refresh rate
- ⚠️ Aumenta input lag ligeramente

---

### Opción 2: Triple Buffering

**Modificar `bTripleBuffer`**:
```cpp
constexpr bool bTripleBuffer = true;  // Activar triple buffering
g_d3dpp.BackBufferCount = bTripleBuffer ? 2 : 1;  // 2 back buffers
```

**Ventajas**:
- ✅ Reduce stuttering cuando VSync está activo
- ✅ Permite que el GPU renderice mientras espera el refresh rate
- ✅ Mejor rendimiento con VSync activo

**Desventajas**:
- ⚠️ Mayor uso de memoria (2 back buffers en lugar de 1)
- ⚠️ No elimina tearing si VSync está desactivado

---

### Opción 3: Cambiar Swap Effect

**Modificar `SwapEffect`**:
```cpp
g_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;  // En lugar de DISCARD
```

**Ventajas**:
- ✅ Mejor para evitar tearing
- ✅ Más control sobre la presentación

**Desventajas**:
- ⚠️ Requiere más memoria
- ⚠️ Puede no estar disponible en todos los modos (especialmente windowed)

---

## 📊 Comparación de Técnicas

| Técnica | Estado | Efectividad Anti-Tearing | Impacto Rendimiento | Input Lag |
|---------|--------|-------------------------|---------------------|-----------|
| **VSync** | ❌ Desactivado | 🟢 Alta | 🟡 Medio (limita FPS) | 🟡 Aumenta |
| **Triple Buffering** | ❌ Desactivado | 🟡 Media (solo con VSync) | 🟢 Bajo | 🟢 Bajo |
| **Swap Effect DISCARD** | ✅ Activado | 🔴 Baja | 🟢 Alto | 🟢 Bajo |
| **Doble Buffering** | ✅ Activado | 🟡 Media (solo con VSync) | 🟢 Bajo | 🟢 Bajo |

---

## 🎯 Recomendaciones

### Para Eliminar Tearing Completamente

1. **Activar VSync**:
   ```cpp
   SetVSync(true);
   ```

2. **Opcional: Activar Triple Buffering** (mejora rendimiento con VSync):
   ```cpp
   constexpr bool bTripleBuffer = true;
   ```

3. **Considerar cambiar Swap Effect** (si es necesario):
   ```cpp
   g_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
   ```

### Para Máximo Rendimiento (Sin VSync)

- Mantener configuración actual (VSync desactivado)
- Aceptar posible tearing a cambio de mayor FPS

---

## 🔍 Búsqueda de Llamadas a SetVSync

**Resultado**: No se encontraron llamadas a `SetVSync()` en el código base.

**Conclusión**: VSync **nunca se activa** automáticamente. El usuario debe activarlo manualmente o a través de configuración.

---

## 📝 Notas Adicionales

### DirectX 9 Presentation Intervals

- `D3DPRESENT_INTERVAL_DEFAULT` (0): Usa el intervalo por defecto del driver
- `D3DPRESENT_INTERVAL_ONE` (1): Sincroniza con cada refresh (VSync ON)
- `D3DPRESENT_INTERVAL_TWO` (2): Sincroniza con cada segundo refresh (VSync a 30Hz en monitor 60Hz)
- `D3DPRESENT_INTERVAL_THREE` (3): Sincroniza con cada tercer refresh
- `D3DPRESENT_INTERVAL_FOUR` (4): Sincroniza con cada cuarto refresh
- `D3DPRESENT_INTERVAL_IMMEDIATE` (0x80000000): Presenta inmediatamente (VSync OFF)

### Vulkan

RealSpace2 también soporta Vulkan, que tiene su propio sistema de presentación:
- Usa `VulkanSwapChain` con semáforos de sincronización
- Soporta VSync a través de `VkPresentModeKHR`
- Más control sobre la presentación que DirectX 9

---

## ✅ Conclusión

**RealSpace2 SÍ implementa VSync**, pero:
- ❌ **Está desactivado por defecto**
- ✅ **Puede activarse llamando `SetVSync(true)`**
- ⚠️ **No hay configuración automática** - debe activarse manualmente

**Para evitar tearing**, se recomienda:
1. Activar VSync llamando `SetVSync(true)`
2. Opcionalmente activar triple buffering para mejor rendimiento
3. Considerar cambiar Swap Effect si es necesario

