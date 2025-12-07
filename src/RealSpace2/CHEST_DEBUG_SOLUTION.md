# Diagnóstico: Chests No Dan Items - Sin Mensajes

## Problema Reportado

Cuando recoges un chest en quest:
- ❌ **NO sale ningún mensaje** (ni de quest ni de item obtenido)
- ❌ **NO se entrega ningún objeto** de la drop table

## Diagnóstico Agregado

He agregado logs de debug detallados en `MatchServer/MMatchRuleQuest.cpp:945` para identificar exactamente qué está pasando.

### Logs Agregados

Cuando recojas un chest, deberías ver en los logs del servidor:

```
OnObtainWorldItem - ItemID: X, CombatState: Y
🔍 CHEST RECOGIDO - ItemID: 51, ExtraValues[0]: X, ExtraValues[1]: Y
```

### Posibles Escenarios

**Escenario 1: No se llama la función**
- Si NO ves ningún log = La función `OnObtainWorldItem()` no se está llamando
- Posibles causas:
  - El chest no se está registrando correctamente cuando se spawnea
  - El cliente no está enviando la petición correctamente

**Escenario 2: CombatState incorrecto**
- Si ves: `⚠️ OnObtainWorldItem: CombatState no es PLAY, retornando`
- El quest no está en estado de juego, retorna sin procesar

**Escenario 3: Chest sin item (MÁS PROBABLE)**
- Si ves: `❌ PROBLEMA: Chest recogido SIN item asignado! ExtraValues[0]=0`
- El chest se spawneó sin item en los ExtraValues
- **Este es el problema más probable**

## Qué Hacer Ahora

1. **Compila y ejecuta el servidor** con los nuevos logs
2. **Recoge un chest** durante una quest
3. **Revisa los logs del servidor** y busca los mensajes que empiezan con:
   - `OnObtainWorldItem`
   - `🔍 CHEST RECOGIDO`
   - `❌ PROBLEMA`
   - `⚠️`

4. **Comparte los logs** que aparezcan para identificar el problema exacto

## Soluciones Según el Problema

### Si el Chest NO tiene item (ExtraValues[0] = 0)

El problema es que los chests se spawnean sin items. Necesitas:

**Opción A: Configurar drop table para chests del mapa**
- Los chests estáticos del mapa necesitan tener una drop table asignada
- Modificar el código para hacer roll cuando se recoge

**Opción B: Verificar cómo se spawnean los chests**
- Si vienen de NPCs, deberían tener items
- Si son estáticos del mapa, necesitan configuración de drop table

### Si la función NO se llama

Verificar:
- Que el chest esté siendo spawneado correctamente
- Que el cliente esté enviando la petición
- Que el servidor esté recibiendo la petición

## Código Actualizado

He agregado logs detallados en `MatchServer/MMatchRuleQuest.cpp` que mostrarán:
- Si la función se llama
- El estado del combate
- Los valores de ExtraValues
- Si se procesa o se retorna

Compila el servidor y prueba de nuevo, luego revisa los logs para ver exactamente qué está pasando.

