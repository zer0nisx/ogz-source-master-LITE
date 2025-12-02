# 🧪 Guía para Probar las Correcciones de OnKnockback

## 📋 Resumen
Esta guía explica cómo probar las correcciones aplicadas a `OnKnockback` usando una katana.

---

## ✅ Correcciones a Probar

1. **Límite de velocidad consistente en modo Blast** (ZMyCharacter)
2. **Eliminación del filtro IsHero()** (ZCharacter)
3. **Documentación en ZActor** (ya estaba correcto)

---

## 🎮 Condiciones Necesarias

### ⚠️ Modos de Juego que DESACTIVAN Knockback

El knockback **NO se aplica** en estos casos:

1. **Skillmap**: `MMATCH_GAMETYPE_SKILLMAP`
   - El código retorna inmediatamente sin aplicar knockback

2. **Training con NoStuns**: `MMATCH_GAMETYPE_TRAINING` + `TrainingSettings.NoStuns`
   - El código retorna sin aplicar knockback

### ✅ Modos de Juego Válidos para Probar

- **Deathmatch** (normal)
- **Team Deathmatch**
- **Training** (sin NoStuns activado)
- **Quest** (para probar con NPCs)

---

## 🗡️ Cómo Probar con Katana

### Método 1: Prueba Básica (Un Jugador)

1. **Preparación:**
   - Inicia el juego en un modo válido (no skillmap)
   - Equipa una katana
   - Si estás en Training, asegúrate de que `NoStuns` esté **desactivado**

2. **Prueba:**
   - Busca otro jugador o NPC
   - Golpéalo con la katana (ataque melee)
   - **Observa:**
     - ✅ El personaje golpeado debería ser **empujado hacia atrás**
     - ✅ Debería haber un **efecto visual de "temblor"** (shake de cámara)
     - ✅ El personaje debería moverse en la dirección del golpe

3. **Verificación de la Corrección 2 (IsHero):**
   - Si golpeas a **otro jugador** (no tu personaje), debería recibir knockback
   - **Antes de la corrección**: Otros jugadores no recibían knockback
   - **Después de la corrección**: Todos los jugadores reciben knockback

### Método 2: Prueba en Modo Blast

1. **Preparación:**
   - Inicia el juego
   - Equipa una katana
   - Necesitas estar en estado **Blast** o **BlastFall**

2. **Cómo entrar en Blast:**
   - Recibe daño de explosión (granada, rocket)
   - O usa un dash attack que te ponga en Blast

3. **Prueba:**
   - Mientras estás en Blast, recibe un golpe de katana
   - **Observa:**
     - ✅ El knockback debería aplicarse con límite de **1700** (no 1000)
     - ✅ Deberías ser empujado más lejos que antes de la corrección

### Método 3: Prueba con NPCs

1. **Preparación:**
   - Inicia un modo Quest
   - Encuentra un NPC (ZActor)

2. **Prueba:**
   - Golpea al NPC con katana
   - **Observa:**
     - ✅ El NPC debería recibir knockback
     - ✅ El NPC debería ser empujado hacia atrás
     - ✅ El NPC debe tener el flag `AF_MY_CONTROL` activado

---

## 🔍 Qué Observar Visualmente

### Efectos Visuales del Knockback

1. **Movimiento del Personaje:**
   - El personaje golpeado se mueve en la dirección opuesta al atacante
   - La velocidad de movimiento debería ser visible

2. **Efecto Tremble (Temblor):**
   - La cámara del personaje golpeado debería "temblar"
   - Este efecto viene de `Tremble(fMaxValue, 50, 100)` en `ZCharacterObject::OnKnockback`

3. **Límite de Velocidad:**
   - El personaje no debería moverse más rápido que el límite máximo
   - Límite normal: **1700 unidades**
   - En Blast (después de corrección): **1700 unidades** (antes era 1000)

---

## 🐛 Debugging y Verificación Técnica

### Opción 1: Agregar Logs Temporales

Puedes agregar logs temporales para verificar que el código se ejecuta:

**En `ZCharacter::OnKnockback`:**
```cpp
void ZCharacter::OnKnockback(const rvector& dir, float fForce)
{
    // Log temporal para debugging
    #ifdef _DEBUG
    MLog("ZCharacter::OnKnockback - Force: %.2f, IsHero: %d\n", fForce, IsHero());
    #endif
    
    ZCharacterObject::OnKnockback(dir, fForce);
}
```

**En `ZMyCharacter::OnKnockback` (modo Blast):**
```cpp
if (m_bBlast || m_bBlastFall) {
    // ... código ...
    
    #ifdef _DEBUG
    float finalMagnitude = Magnitude(vel);
    MLog("ZMyCharacter::OnKnockback (Blast) - Final velocity magnitude: %.2f (limit: 1700)\n", finalMagnitude);
    #endif
    
    SetVelocity(vel);
}
```

### Opción 2: Verificar Velocidad en Tiempo Real

Puedes agregar código para mostrar la velocidad actual:

```cpp
// En algún lugar donde puedas verlo (por ejemplo, en ZMyCharacter::OnUpdate)
#ifdef _DEBUG
if (Magnitude(GetVelocity()) > 500.f) {
    MLog("High velocity detected: %.2f\n", Magnitude(GetVelocity()));
}
#endif
```

### Opción 3: Usar el Debugger

1. Coloca un **breakpoint** en:
   - `ZCharacter::OnKnockback` (línea 1541)
   - `ZMyCharacter::OnKnockback` (línea 2864)
   - `ZCharacterObject::OnKnockback` (línea 550)

2. Cuando se ejecute, verifica:
   - El valor de `fForce` (debería ser ~200.0 para katana)
   - El valor de `IsHero()` (debería ser true para ZMyCharacter)
   - La magnitud de la velocidad después del knockback

---

## 📊 Valores Esperados

### Fuerza de Knockback de Katana

Según `ZItem::GetKnockbackForce()`:
- **Katana (melee)**: `200.0f`
- Este valor se pasa a `OnKnockback(dir, 200.0f)`

### Límites de Velocidad

- **Límite normal**: `MAX_KNOCKBACK_VELOCITY = 1700.f`
- **Límite en Blast (después de corrección)**: `1700.f` (antes era 1000.f)
- **Límite de movimiento normal**: `MAX_SPEED = 1000.f`

### Factor de Knockback en Blast

- **BLASTED_KNOCKBACK_RATIO**: `3.0f`
- En Blast, la fuerza se multiplica por 3: `fForce * 3.0`
- Componentes X e Y se reducen a 0.2: `vKnockBackDir.x * 0.2f`

---

## ✅ Checklist de Pruebas

### Prueba 1: Knockback Normal (Sin Blast)
- [ ] El personaje golpeado se mueve hacia atrás
- [ ] Hay efecto visual de temblor
- [ ] La velocidad no excede 1700 unidades
- [ ] Funciona tanto para tu personaje como para otros jugadores

### Prueba 2: Knockback en Modo Blast
- [ ] El personaje en Blast recibe knockback
- [ ] El límite de velocidad es 1700 (no 1000)
- [ ] El knockback es más fuerte (x3) pero limitado correctamente

### Prueba 3: Knockback en NPCs
- [ ] Los NPCs reciben knockback cuando son golpeados
- [ ] El knockback funciona correctamente

### Prueba 4: Verificación de Corrección IsHero
- [ ] Otros jugadores (no-Hero) ahora reciben knockback
- [ ] Antes de la corrección no recibían knockback
- [ ] Después de la corrección sí reciben knockback

---

## 🚨 Problemas Comunes

### "No veo ningún knockback"

**Posibles causas:**
1. Estás en modo **Skillmap** → Cambia a otro modo
2. Estás en **Training con NoStuns** → Desactiva NoStuns
3. El personaje está **muerto** → El knockback no se aplica a muertos
4. El personaje está **invulnerable** → Verifica `isInvincible()`

### "El knockback es muy débil"

**Verifica:**
- La fuerza de knockback de la katana es 200.0f
- Si estás en Blast, debería ser más fuerte (x3)
- El límite de velocidad podría estar limitando el efecto

### "Otros jugadores no reciben knockback"

**Antes de la corrección:**
- Esto era normal (problema del filtro IsHero)

**Después de la corrección:**
- Deberían recibir knockback
- Si no lo reciben, verifica que la corrección se compiló correctamente

---

## 📝 Notas Adicionales

### Orden de Ejecución

Cuando golpeas con katana:
1. `ZGame::OnPeerSlash()` o `ZGame::OnPeerShot_Melee()` detecta el golpe
2. Calcula `fKnockbackForce = pItem->GetKnockbackForce()` (200.0 para katana)
3. Llama `pTarget->OnKnockback(dir, fKnockbackForce)`
4. Si es `ZMyCharacter`: ejecuta `ZMyCharacter::OnKnockback()`
5. Si es otro `ZCharacter`: ejecuta `ZCharacter::OnKnockback()` (ahora sin filtro IsHero)
6. Si es `ZActor`: ejecuta `ZActor::OnKnockback()` (solo si AF_MY_CONTROL)

### Puntos de Invocación

- **Línea 2464** (`ZGame.cpp`): `OnPeerShot_Melee()` - Golpe melee directo
- **Línea 2598** (`ZGame.cpp`): `OnPeerSlash()` - Slash normal
- **Línea 2807** (`ZGame.cpp`): `DoOneShot()` - Disparos de balas

---

**¡Buena suerte con las pruebas!** 🎮






