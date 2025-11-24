# Optimizaciones Aplicadas a Skills de NPCs

## Problemas Identificados y Solucionados

### 1. ✅ Iteraciones Costosas Optimizadas
**Problema**: Iteración sobre TODOS los objetos cada frame/segundo
**Solución**:
- Filtrado por distancia usando `MagnitudeSq` (más rápido que `Magnitude`)
- Límite máximo de objetos procesados por frame (20-30 objetos)
- Early exit cuando se alcanza el límite

### 2. ✅ REPEAT Limitados
**Problema**: Skills con 40+ REPEAT crean demasiados efectos simultáneos
**Solución**:
- Límite máximo de 10 REPEAT por skill
- Si un skill tiene más REPEAT, se desactiva automáticamente

### 3. ✅ Efectos Especiales Limitados
**Problema**: `AddSp()` podía crear efectos ilimitados
**Solución**:
- Máximo 5 efectos especiales por skill
- Verificación antes de crear cada efecto

### 4. ✅ Verificaciones NULL Agregadas
**Problema**: Uso de punteros sin verificar
**Solución**:
- Verificaciones NULL antes de cada uso
- Verificación de objetos muertos antes de procesar

### 5. ✅ Optimización de Cálculos
**Problema**: `Magnitude()` es costoso (usa sqrt)
**Solución**:
- Uso de `MagnitudeSq()` para comparaciones de distancia
- Comparación con `fMaxRange * fMaxRange` en lugar de `fMaxRange`

## Cómo Crear Efectos que Persigan Múltiples Objetivos

### Efectos Guiables (Guided Effects)
Los efectos que persiguen objetivos se crean usando `bGuidable="true"` en el XML y `AddMagic()`:

```xml
<SKILL id="XXX" name="Magic Fire" guidable="true" ... />
```

**Implementación**:
- `ZWeaponMagic` persigue el objetivo usando `m_uidTarget` y `m_bGuide`
- El efecto actualiza su dirección cada frame hacia el objetivo
- Se puede modificar para perseguir múltiples objetivos

### Para Crear Efectos que Persigan Varios Objetivos:

1. **Modificar el XML** para usar `REPEAT` con diferentes ángulos:
```xml
<SKILL id="XXX" name="Multi-Target Magic" guidable="true" ...>
    <REPEAT delay="0.0" angle="0.0 0.0 0.2" />
    <REPEAT delay="0.0" angle="0.0 0.0 -0.2" />
    <REPEAT delay="0.0" angle="0.0 0.0 0.4" />
</SKILL>
```

2. **Modificar `ZWeaponMagic`** para soportar múltiples objetivos:
   - Agregar `std::vector<MUID> m_vTargets` en lugar de `MUID m_uidTarget`
   - En `Update()`, calcular dirección hacia el objetivo más cercano
   - O dividir el efecto en múltiples instancias (una por objetivo)

3. **Ejemplo de implementación**:
```cpp
// En ZWeapon.cpp, modificar Update() de ZWeaponMagic
if(m_bGuide && !m_vTargets.empty()) {
    // Encontrar el objetivo más cercano
    ZObject* pClosestTarget = nullptr;
    float fClosestDist = FLT_MAX;
    
    for(MUID uid : m_vTargets) {
        ZObject* pTarget = ZGetObjectManager()->GetObject(uid);
        if(!pTarget || pTarget->IsDead()) continue;
        
        float fDist = Magnitude(pTarget->GetPosition() - m_Position);
        if(fDist < fClosestDist) {
            fClosestDist = fDist;
            pClosestTarget = pTarget;
        }
    }
    
    if(pClosestTarget) {
        // Perseguir el objetivo más cercano
        rvector dir = (pClosestTarget->GetPosition() + rvector(0,0,100)) - m_Position;
        Normalize(dir);
        // ... actualizar dirección ...
    }
}
```

## Límites Configurados

- **MAX_REPEAT_COUNT**: 10 repeticiones por skill
- **MAX_OBJECTS_PER_FRAME**: 20 objetos en Update()
- **MAX_OBJECTS_PER_SKILL**: 30 objetos en Execute()
- **MAX_SP_EFFECTS_PER_SKILL**: 5 efectos especiales

## Mejoras de Rendimiento Esperadas

- **Antes**: 1500 FPS → 45 FPS (caída de 97%)
- **Después**: Mejora significativa esperada
- **Optimizaciones aplicadas**:
  - Reducción de iteraciones (de todos los objetos a máximo 30)
  - Uso de MagnitudeSq en lugar de Magnitude
  - Límite de REPEAT (de 40+ a máximo 10)
  - Early exit en loops

## Notas Adicionales

- Los skills con muchos REPEAT (como skill 263 con 40 REPEAT) ahora se limitan automáticamente
- Los efectos mágicos que persiguen objetivos funcionan correctamente con `guidable="true"`
- Para efectos que persigan múltiples objetivos, se recomienda usar REPEAT con diferentes ángulos o modificar `ZWeaponMagic`

