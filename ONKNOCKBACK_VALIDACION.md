# ✅ Validación del Ciclo de Vida de OnKnockback

## 📋 Resumen Ejecutivo
Se validaron los problemas identificados inicialmente en el ciclo de vida de `OnKnockback`. 

**Resultado de la validación:**
1. ⚠️ **Problema 1 (Límite de velocidad en Blast)**: PARCIALMENTE REAL - Hay límite, pero es inconsistente
2. ✅ **Problema 2 (Filtro IsHero)**: REAL - Confirmado que es restrictivo

---

## 🔍 Problema 1: Límite de Velocidad en Modo Blast

### Código Analizado
**Ubicación:** `ZMyCharacter.cpp:2874-2880`

```cpp
if (m_bBlast || m_bBlastFall) {
    rvector vKnockBackDir = dir;
    Normalize(vKnockBackDir);
    vKnockBackDir *= (fForce * BLASTED_KNOCKBACK_RATIO);  // x3 la fuerza
    vKnockBackDir.x = vKnockBackDir.x * 0.2f;  // Reduce componente X
    vKnockBackDir.y = vKnockBackDir.y * 0.2f;  // Reduce componente Y
    SetVelocity(vKnockBackDir);  // Aplica directamente
}
```

### Validación Realizada

1. **Verificación de límites aplicados:**
   - ✅ `UpdateVelocity()` se ejecuta después (línea 1599 en `ZMyCharacter::OnUpdate`)
   - ✅ Limita la velocidad horizontal a `MAX_SPEED = 1000.f` (línea 1209 en `ZCharacter::UpdateVelocity`)
   - ✅ `MAX_KNOCKBACK_VELOCITY = 1700.f` NO se aplica directamente en modo Blast

2. **Valores encontrados:**
   - `MAX_KNOCKBACK_VELOCITY = 1700.f` (límite normal de knockback)
   - `MAX_SPEED = 1000.f` (límite de velocidad de movimiento normal)
   - `BLASTED_KNOCKBACK_RATIO = 3.f` (multiplica la fuerza por 3)
   - `RUN_SPEED = 630.f` (velocidad de carrera normal)

3. **Flujo de ejecución:**
   ```
   OnKnockback() -> SetVelocity(vKnockBackDir)
                      ↓
   [En el siguiente frame]
   OnUpdate() -> UpdateVelocity()
                      ↓
   Limita velocidad horizontal a MAX_SPEED (1000)
   ```

### Conclusión

**✅ HAY límite aplicado**, pero:
- ⚠️ El límite es **1000** en lugar de **1700** (más restrictivo)
- ⚠️ Solo se aplica al componente horizontal (X, Y), no a Z
- ⚠️ Se aplica en el siguiente frame, no inmediatamente

**Esto puede ser diseño intencional** para que el knockback en Blast sea más controlado. Sin embargo, hay inconsistencia con el límite usado en knockback normal.

---

## 🔍 Problema 2: Filtro IsHero() Restrictivo

### Código Analizado
**Ubicación:** `ZCharacter.cpp:1541-1545`

```cpp
void ZCharacter::OnKnockback(const rvector& dir, float fForce)
{
    if(IsHero())  // Solo aplica si es "Hero"
        ZCharacterObject::OnKnockback(dir, fForce);
}
```

### Validación Realizada

1. **Verificación de cuándo se setea IsHero():**
   - ✅ `ZMyCharacter` setea `m_bHero = true` en línea 2209
   - ✅ `ZCharacter` carga `m_bHero = rpi.IsHero` en línea 2699
   - ⚠️ Solo `ZMyCharacter` tiene explícitamente `m_bHero = true`

2. **Jerarquía de clases:**
   ```
   ZCharacter (OnKnockback con filtro IsHero)
     ├─ ZMyCharacter (override final - tiene m_bHero = true)
     └─ Otros personajes (probablemente m_bHero = false)
   ```

3. **Implementaciones alternativas:**
   - `ZActor` tiene su propio `OnKnockback` que NO depende de `IsHero()`
   - `ZMyBotCharacter` llama a `ZCharacter::OnKnockback`, que depende de `IsHero()`

### Conclusión

**✅ PROBLEMA REAL:**
- ⚠️ Personajes que no son "Hero" no reciben knockback desde `ZCharacter::OnKnockback`
- ⚠️ Esto afecta a otros jugadores y potencialmente a bots si no tienen `m_bHero = true`
- ⚠️ Solo `ZMyCharacter` está garantizado de recibir knockback

**Excepciones:**
- `ZActor` (NPCs) tiene su propia implementación que no depende de `IsHero()`
- Pero si un `ZCharacter` que no es `ZMyCharacter` no tiene `m_bHero = true`, no recibirá knockback

---

## 📊 Comparación de Límites de Velocidad

| Situación | Límite Aplicado | Valor | Ubicación |
|-----------|----------------|-------|-----------|
| Knockback Normal | MAX_KNOCKBACK_VELOCITY | 1700.f | ZCharacterObject::OnKnockback |
| Knockback en Blast | MAX_SPEED (vía UpdateVelocity) | 1000.f | ZCharacter::UpdateVelocity |
| Velocidad de Carrera | RUN_SPEED | 630.f | ZCharacter::UpdateVelocity |
| Velocidad Máxima | MAX_SPEED | 1000.f | ZCharacter::UpdateVelocity |

**Observación:** En modo Blast, el límite es más restrictivo (1000 vs 1700), pero puede ser intencional.

---

## 🔄 Flujo Completo Validado

```
1. Ataque impacta
   └─> ZGame::OnPeerShot_Melee() / DoOneShot() / OnPeerSlash()
        └─> pTarget->OnKnockback(dir, force)

2. Si es ZMyCharacter:
   └─> ZMyCharacter::OnKnockback() (override final)
        ├─> Validación: MMATCH_GAMETYPE_SKILLMAP? → return
        ├─> Validación: TRAINING + NoStuns? → return
        ├─> Si (m_bBlast || m_bBlastFall):
        │   ├─> Calcula: dir * fForce * BLASTED_KNOCKBACK_RATIO (x3)
        │   ├─> Reduce X, Y a 0.2
        │   ├─> SetVelocity(vKnockBackDir)
        │   └─> [Frame siguiente] UpdateVelocity() limita a MAX_SPEED (1000)
        └─> Si no Blast:
             └─> ZCharacter::OnKnockback()
                  └─> Si IsHero(): ✅ (ZMyCharacter siempre es Hero)
                      └─> ZCharacterObject::OnKnockback()
                          ├─> AddVelocity(dir * fForce)
                          ├─> Limita a MAX_KNOCKBACK_VELOCITY (1700)
                          └─> Tremble(efecto visual)

3. Si es ZCharacter (no ZMyCharacter):
   └─> ZCharacter::OnKnockback()
        └─> Si IsHero(): ❌ (probablemente false)
            └─> NO ejecuta nada (problema real)

4. Si es ZActor (NPC):
   └─> ZActor::OnKnockback()
        ├─> Si CheckFlag(AF_MY_CONTROL):
        └─> ZCharacterObject::OnKnockback()
            └─> Funciona normalmente (no depende de IsHero)
```

---

## ✅ Recomendaciones Finales

### Problema 1 (Límite inconsistente):
1. **Decisión de diseño**: Determinar si el límite más restrictivo en Blast (1000 vs 1700) es intencional
2. **Si es intencional**: Documentar la razón en el código
3. **Si no es intencional**: Aplicar `MAX_KNOCKBACK_VELOCITY` también en modo Blast

### Problema 2 (Filtro IsHero):
1. **Revisar necesidad del filtro**: ¿Por qué solo "Hero" recibe knockback?
2. **Opciones de corrección**:
   - Opción A: Eliminar el filtro `if(IsHero())` si todos deberían recibir knockback
   - Opción B: Cambiar la condición a algo más inclusivo
   - Opción C: Mover la validación a otro nivel donde tenga más sentido
3. **Verificar impacto**: Confirmar si otros jugadores o bots deberían recibir knockback visual/físico

### Recomendación General:
- Documentar las decisiones de diseño detrás de cada límite y filtro
- Considerar unificar el comportamiento para mantener consistencia

---

## 📝 Archivos Relevantes

- `src/Gunz/ZObject.h:121` - Definición base (vacía)
- `src/Gunz/ZCharacterObject.h:63` - Declaración
- `src/Gunz/ZCharacterObject.cpp:550` - Implementación base con límite 1700
- `src/Gunz/ZCharacter.cpp:1541` - Filtro IsHero()
- `src/Gunz/ZMyCharacter.cpp:2864` - Implementación final con manejo Blast
- `src/Gunz/ZMyCharacter.cpp:1599` - UpdateVelocity que limita a 1000
- `src/Gunz/ZActor.cpp:831` - Implementación para NPCs
- `src/Gunz/ZMyBotCharacter.cpp:393` - Implementación para bots
- `src/Gunz/ZGame.cpp:2464, 2598, 2807` - Puntos de invocación
- `src/Gunz/ZCharacter.cpp:2592` - Invocación en ActDead()

---

**Fecha de validación:** Análisis realizado mediante revisión de código fuente completo.

