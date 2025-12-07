# Implementación: Barra de HP Dinámica para NPCs

## Resumen

Se implementará un sistema para mostrar barras de HP dinámicas sobre los NPCs cuando reciben daño. La barra aparecerá automáticamente cuando un NPC recibe daño y se ocultará después de 5 segundos sin recibir daño.

## Archivos a Modificar

1. **Gunz/ZCombatInterface.h** - Agregar estructura de tracking y declaraciones
2. **Gunz/ZCombatInterface.cpp** - Implementar funciones de dibujado y tracking
3. **Gunz/ZActor.cpp** - Notificar cuando un NPC recibe daño

## Funcionalidades

- ✅ Barra de HP aparece cuando NPC recibe daño
- ✅ Barra se oculta después de 5 segundos sin daño
- ✅ Posicionamiento usando GetHeadPosition()
- ✅ Actualización suave del HP
- ✅ Integración con el sistema existente de dibujado

## Estructura de Datos

```cpp
struct NPCHPBarInfo
{
    float fLastDamageTime;  // Tiempo del último daño (en segundos)
    float fCurrentHP;       // HP actual para animación suave
    float fMaxHP;           // HP máximo
};

std::map<MUID, NPCHPBarInfo> m_NPCHPBarMap;
```

## Funciones a Implementar

1. `NotifyNPCDamaged(MUID uidNPC)` - Notificar que un NPC recibió daño
2. `UpdateNPCHPBars(float fDeltaTime)` - Actualizar timers y limpiar barras expiradas
3. `DrawNPCHPBar(MDrawContext* pDC)` - Dibujar las barras de HP

## Flujo

1. NPC recibe daño → `ZActor::OnDamaged()` → `NotifyNPCDamaged()`
2. Cada frame → `UpdateNPCHPBars()` → Limpia barras expiradas (>5s)
3. En OnDraw() → `DrawNPCHPBar()` → Dibuja barras activas

