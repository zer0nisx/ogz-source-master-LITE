# Análisis del FSM (Finite State Machine) - Mejoras Propuestas

## Estado Actual del FSM

### Problemas Identificados

1. **Estados Vacíos**
   - `ZBehavior_Idle`, `ZBehavior_Attack`, `ZBehavior_Patrol`, `ZBehavior_Script` están completamente vacíos
   - No tienen lógica implementada en `OnEnter()`, `OnExit()`, `OnRun()`
   - El FSM existe pero no se está utilizando activamente

2. **Transiciones Limitadas**
   - Solo hay UNA transición definida: `IDLE -> ATTACK` cuando recibe `ZBEHAVIOR_INPUT_ATTACKED`
   - No hay transiciones de vuelta (ATTACK -> IDLE cuando pierde el objetivo)
   - Estados como PATROL, RETREAT, SCRIPT no están conectados

3. **Inputs Insuficientes**
   - Solo existe `ZBEHAVIOR_INPUT_ATTACKED`
   - Faltan inputs como:
     - `ZBEHAVIOR_INPUT_TARGET_FOUND`
     - `ZBEHAVIOR_INPUT_TARGET_LOST`
     - `ZBEHAVIOR_INPUT_LOW_HEALTH`
     - `ZBEHAVIOR_INPUT_PATH_BLOCKED`
     - `ZBEHAVIOR_INPUT_SKILL_READY`

4. **Bug en ZStateMachine::DeleteState()**
   - Elimina el estado pero NO lo remueve del `m_StateMap`
   - Puede causar memory leaks y acceso a punteros inválidos

5. **Falta de Validación**
   - No se valida si una transición es válida antes de ejecutarla
   - No hay verificación de que el estado destino existe

6. **No se Integra con ZBrain**
   - El FSM no se está usando en la lógica principal de `ZBrain`
   - `ZBrain::Think()` no envía inputs al FSM basado en condiciones del juego

7. **Falta de Estados Intermedios**
   - No hay estados para "perseguir", "huir", "buscar", "esperar"
   - El comportamiento es muy básico

## Mejoras Propuestas

### 1. Corregir Bug en DeleteState
```cpp
void ZStateMachine::DeleteState(int nStateID)
{
    map<int, ZState*>::iterator itor = m_StateMap.find(nStateID);
    if (itor != m_StateMap.end())
    {
        delete (*itor).second;
        m_StateMap.erase(itor);  // FALTA ESTA LÍNEA
    }
}
```

### 2. Agregar Más Inputs al FSM
```cpp
enum ZBEHAVIOR_INPUT 
{
    ZBEHAVIOR_INPUT_NONE = 0,
    ZBEHAVIOR_INPUT_ATTACKED,        // Recibe daño
    ZBEHAVIOR_INPUT_TARGET_FOUND,   // Encuentra objetivo
    ZBEHAVIOR_INPUT_TARGET_LOST,    // Pierde objetivo
    ZBEHAVIOR_INPUT_LOW_HEALTH,     // Salud baja
    ZBEHAVIOR_INPUT_PATH_BLOCKED,   // Camino bloqueado
    ZBEHAVIOR_INPUT_SKILL_READY,    // Skill listo
    ZBEHAVIOR_INPUT_TARGET_IN_RANGE, // Objetivo en rango
    ZBEHAVIOR_INPUT_TARGET_OUT_RANGE, // Objetivo fuera de rango
    ZBEHAVIOR_INPUT_END
};
```

### 3. Implementar Lógica en los Estados

#### ZBehavior_Idle
- Buscar objetivos cercanos
- Si encuentra objetivo, enviar `ZBEHAVIOR_INPUT_TARGET_FOUND`
- Si tiene patrulla definida, cambiar a PATROL después de un tiempo

#### ZBehavior_Attack
- Verificar si el objetivo sigue siendo válido
- Si pierde el objetivo, enviar `ZBEHAVIOR_INPUT_TARGET_LOST`
- Si el objetivo está fuera de rango, enviar `ZBEHAVIOR_INPUT_TARGET_OUT_RANGE`
- Coordinar con `ZBrain::ProcessAttack()`

#### ZBehavior_Patrol
- Mover entre waypoints de patrulla
- Si encuentra objetivo, enviar `ZBEHAVIOR_INPUT_TARGET_FOUND`
- Volver a IDLE cuando termine la patrulla

### 4. Integrar FSM con ZBrain

En `ZBrain::Think()`:
```cpp
void ZBrain::Think(float fDelta)
{
    // Actualizar FSM
    m_Behavior.Run(fDelta);
    
    // Enviar inputs al FSM basado en condiciones
    ZObject* pTarget = GetTarget();
    if (pTarget)
    {
        if (m_Behavior.GetCurrStateID() == ZBEHAVIOR_STATE_IDLE)
            m_Behavior.Input(ZBEHAVIOR_INPUT_TARGET_FOUND);
    }
    else
    {
        if (m_Behavior.GetCurrStateID() == ZBEHAVIOR_STATE_ATTACK)
            m_Behavior.Input(ZBEHAVIOR_INPUT_TARGET_LOST);
    }
    
    // Resto de la lógica...
}
```

### 5. Agregar Validación de Transiciones

```cpp
bool ZStateMachine::CanTransition(int nInput, int nTargetState) const
{
    ZState* pState = GetState(m_nCurrState);
    if (!pState) return false;
    
    int nextState = pState->GetOutput(nInput);
    if (nextState == INVALID_STATE) return false;
    
    // Verificar que el estado destino existe
    return (m_StateMap.find(nTargetState) != m_StateMap.end());
}
```

### 6. Agregar Callbacks de Estado

```cpp
class ZBehaviorState
{
protected:
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnRun(float fDelta) {}
    virtual bool CanEnter() const { return true; }  // Validación antes de entrar
    virtual bool CanExit() const { return true; }   // Validación antes de salir
};
```

### 7. Mejorar el Sistema de Transiciones

Permitir transiciones condicionales:
```cpp
class ZState
{
    struct Transition
    {
        int nInput;
        int nOutputID;
        std::function<bool()> condition;  // Condición opcional
    };
    
    bool AddTransition(int nInput, int nOutputID, std::function<bool()> condition = nullptr);
};
```

## Prioridades de Implementación

1. **ALTA**: Corregir bug en `DeleteState()`
2. **ALTA**: Agregar más inputs y transiciones básicas
3. **MEDIA**: Implementar lógica en `ZBehavior_Idle` y `ZBehavior_Attack`
4. **MEDIA**: Integrar FSM con `ZBrain::Think()`
5. **BAJA**: Agregar validación de transiciones
6. **BAJA**: Implementar estados avanzados (PATROL, RETREAT)

## Beneficios Esperados

1. **Mejor Organización**: El comportamiento del NPC estará claramente definido por estados
2. **Más Flexible**: Fácil agregar nuevos comportamientos sin modificar código existente
3. **Más Predecible**: El comportamiento será más consistente y fácil de depurar
4. **Mejor Rendimiento**: Evitar verificaciones innecesarias cuando el estado no lo requiere
5. **Más Mantenible**: Separación clara de responsabilidades

