# Optimización de Humo y Cartuchos de Armas

## Problema Identificado

El juego experimentaba lag significativo debido a que:
1. **Humo de boca de cañón**: Se creaba en **cada disparo** sin ningún throttling
2. **Cartuchos vacíos**: Se creaban en cada disparo (aunque ya tenían throttling básico)
3. **Armas automáticas**: Con alta cadencia de fuego, generaban cientos de efectos por segundo

## Solución Implementada

### 1. Throttling de Humo de Boca de Cañón

**Antes**: El humo se creaba en cada disparo sin control.

**Ahora**: El humo respeta el nivel de efectos configurado:

- **Alto (HIGH)**: 
  - Armas normales: Siempre renderiza
  - Armas automáticas (MWT_MACHINEGUN): Solo cada 2 disparos (reduce 50% de efectos)
  
- **Normal**: 
  - Armas normales: Cada 2 disparos (toggle)
  - Armas automáticas: Solo cada 3 disparos (reduce 66% de efectos)
  
- **Bajo (LOW)**: 
  - Todas las armas: Solo cada 4 disparos (reduce 75% de efectos)

### 2. Optimización de Cartuchos

**Mejoras aplicadas**:
- **Alto (HIGH)**: 
  - Armas automáticas: Solo cada 2 disparos (reduce 50% de cartuchos)
  
- **Normal**: 
  - Armas automáticas: Solo cada 3 disparos (reduce 66% de cartuchos)
  
- **Bajo (LOW)**: 
  - Todas las armas: Solo cada 4 disparos (reduce 75% de cartuchos)

### 3. Optimización de Escopeta

- **Alto**: Sin cambios (escopeta dispara poco frecuentemente)
- **Normal**: Toggle cada disparo
- **Bajo**: Solo cada 2 disparos

## Impacto en Rendimiento

### Reducción de Efectos por Segundo

**Escenario**: Jugador con ametralladora disparando a 10 balas/segundo

**Antes**:
- Humo: 10 efectos/segundo
- Cartuchos: 10 efectos/segundo (HIGH), 5 efectos/segundo (NORMAL)
- **Total**: 15-20 efectos/segundo

**Después (NORMAL)**:
- Humo: ~3.3 efectos/segundo (cada 3 disparos)
- Cartuchos: ~3.3 efectos/segundo (cada 3 disparos)
- **Total**: ~6.6 efectos/segundo
- **Reducción**: ~66% menos efectos

**Después (LOW)**:
- Humo: 2.5 efectos/segundo (cada 4 disparos)
- Cartuchos: 2.5 efectos/segundo (cada 4 disparos)
- **Total**: 5 efectos/segundo
- **Reducción**: ~75% menos efectos

## Archivos Modificados

- `src/Gunz/ZEffectManager.cpp`:
  - `AddShotEffect()`: Agregado throttling para humo de boca de cañón
  - `AddShotEffect()`: Mejorado throttling para cartuchos
  - `AddShotgunEffect()`: Agregado throttling para cartuchos de escopeta

## Configuración

El nivel de efectos se controla mediante `g_nEffectLevel`:
- `Z_VIDEO_EFFECT_HIGH`: Máxima calidad, throttling mínimo
- `Z_VIDEO_EFFECT_NORMAL`: Calidad media, throttling moderado
- `Z_VIDEO_EFFECT_LOW`: Calidad baja, throttling agresivo

Este nivel se puede configurar desde las opciones de video del juego.

## Notas Técnicas

1. **Contadores estáticos**: Se usan contadores estáticos para el throttling de armas automáticas. Esto es aceptable porque el throttling es por tipo de arma, no por jugador.

2. **Compatibilidad**: Los cambios son compatibles con el sistema existente de niveles de efectos.

3. **Armas duales**: El throttling también se aplica a armas duales (size == 6).

## Recomendaciones Adicionales

Si el lag persiste, se pueden implementar mejoras adicionales:

1. **Sistema de pooling**: Reutilizar efectos en lugar de crear/destruir constantemente
2. **LOD (Level of Detail)**: Reducir calidad de efectos basado en distancia
3. **Límite de efectos activos**: Limitar el número máximo de efectos simultáneos
4. **Throttling basado en tiempo**: En lugar de contar disparos, usar tiempo mínimo entre efectos

## Pruebas Recomendadas

1. Probar con múltiples jugadores disparando simultáneamente
2. Probar con armas automáticas en combates intensos
3. Verificar que los efectos aún se ven bien visualmente
4. Medir FPS antes y después de los cambios

