# Análisis de Singletons y Accesores en el Código Fuente de Gunz

Si has estudiado arquitectura de software durante algún tiempo, probablemente hayas leído una larga lista de razones por las que los singletons deberían ser una opción cuidadosamente considerada en lugar de un botón de "oh, mierda, necesito esto ahora", como se usan tan a menudo. A pesar de que el concepto es simple, vienen con desventajas que a menudo se ignoran. A menos que te encuentres en una situación en la que un singleton sea casi inevitable, como en los ganchos donde no controlas los argumentos, más vale que tengas una muy buena razón para usar uno.

Los singletons son, simplemente, muy convenientes para el desarrollador en ese momento.

Ahora que he dicho algo positivo sobre los singletons, me gustaría pasar directamente a criticarlos y a su implementación en el código fuente de Gunz.

## Versión 1.0

Echemos un vistazo primero a la versión 1.0:

```cpp
// ZGlobal.cpp - Gunz 1.0
// https://github.com/WhyWolfie/Source2007/blob/324257e9c69bbaec872ebb7ae4f96ab2ce98f520/Gunz/ZGlobal.cpp#L68C1-L71C2

ZGame* ZGetGame(void) {
    return ZApplication::GetGameInterface() ?
        ZApplication::GetGameInterface()->GetGame() : NULL;
}
```

Esto parece bastante simple, ¿verdad? Después de una ejecución repetida en una CPU moderna, esto se convierte en una ruta de código muy predecible, por lo que la sobrecarga debería disminuir gradualmente, ¿verdad?

Bueno, eso depende. ¿Qué demonios hacen realmente `ZApplication::GetGameInterface()` y `ZApplication::GetGameInterface()->GetGame()`? Por cada desreferencia de puntero adicional, hay un punto adicional en el que podría ocurrir un error de caché.

```cpp
ZGame* ZApplication::GetGame(void)
{
    // Potential cache miss, but unlikely. Still complicates prediction.
    return (GetGameInterface()->GetGame());
}
```

Bueno, no es el fin del mundo. Esto también debería ser bastante predecible. La predicción de ramificaciones es fundamental.

Pero ¿qué pasa con el código que genera? Si se llamara a `ZGetGame()` varias veces a lo largo de una función, la mayoría asumiría que el compilador optimizaría esas llamadas redundantes.

```cpp
// IDA pseudo-c decomp
// Start of an inlined ZGetGame call
if ( ZApplication::GetGameInterface() ) {
    v24 = ZApplication::GetGameInterface();
    v74 = ZGameInterface::GetGame(v24);
} else {
    v74 = 0;
}
// end
v25 = ZObject::GetUID(pAttacker);
ZCharacter::SetLastThrower(v74->m_pMyCharacter, *v25, fShotTime + 1.0);
ZPostReaction(fShotTime, 2);
// Start of an inlined ZGetGame call
if ( ZApplication::GetGameInterface() ){
    v26 = ZApplication::GetGameInterface();
    v75 = ZGameInterface::GetGame(v26);
} else {
    v75 = 0;
}
// end
```

**No.** Las llamadas repetidas a `ZGetGameInterface()` son completamente redundantes.

## Versión 1.5

Considere el siguiente código, tomado del código fuente de la versión 1.5:

```cpp
if (pVictim == ZGetGame()->m_pMyCharacter)
{
    ZGetGame()->m_pMyCharacter->SetLastThrower(pAttacker->GetUID(), fShotTime + 1.0f);
    ZPostReaction(fShotTime, 2);
    ZGetGame()->m_pMyCharacter->AddVelocity(rvector(0, 0, 1700));
}
pVictim->OnBlast(AttackerDir);

if (ZGetGame()->CanAttack(pAttacker, pVictim))
    pVictim->OnDamagedSkill(pAttacker, AttackerPos, ZD_MELEE, MWT_DAGGER, m_fDamage, m_fPierce);
```

Sabiendo lo que sabemos ahora, que cada llamada `ZGet*` no será eliminada mágicamente por el compilador a favor del primer resultado devuelto, podemos deducir con seguridad que la forma correcta de implementar la lógica anterior es algo más cercano a esto:

```cpp
auto pGame = ZGetGame();
auto pMyCharacter = pGame->m_pMyCharacter;

if (pVictim == pMyCharacter)
{
    pMyCharacter->SetLastThrower(pAttacker->GetUID(), fShotTime + 1.0f);
    ZPostReaction(fShotTime, 2);
    pMyCharacter->AddVelocity(rvector(0, 0, 1700));
}
pVictim->OnBlast(AttackerDir);

if (pGame->CanAttack(pAttacker, pVictim))
    pVictim->OnDamagedSkill(pAttacker, AttackerPos, ZD_MELEE, MWT_DAGGER, m_fDamage, m_fPierce);
```

Si no te importa la diferencia entre la versión 1.0 y la 1.5, puedes detenerte aquí.

En la versión 1.5, tras muchos años de ser completamente manipulado por mirageofpenguins y compañía, Maiet decidió incorporar un montón de basura. Si una función no tiene límites, no hay una buena manera de engancharla o llamarla directamente desde un truco interno. Por supuesto, este no fue el enfoque mejor planificado. Estos accesores, que se usan en todo el código base, (en su mayoría) solo leen un puntero de `ZApplication::g_pInstance`, así que los desarrolladores de trucos harían precisamente eso.

Echemos un vistazo a `ZGlobal.cpp` de la versión 1.5:

```cpp
// https://github.com/WhyWolfie/Gregon13Source/blob/master/Gunz/ZGlobal.h
// dll-injection으로 호출하는 핵 때문에 매크로 인라이닝
// Translation: Macro inlining due to hack calling with dll-injection

#define ZGetApplication()        ZApplication::GetInstance()
#define ZGetGameClient()        (ZApplication::GetGameInterface() ? ZApplication::GetGameInterface()->GetGameClient() : NULL)
#define ZGetGame()                (ZApplication::GetGameInterface() ? ZApplication::GetGameInterface()->GetGame() : NULL)

#define ZGetGameInterface()        ZApplication::GetGameInterface()
#define ZGetCombatInterface()    (ZApplication::GetGameInterface() ? ZApplication::GetGameInterface()->GetCombatInterface() : NULL)

#define ZGetFileSystem()        ZApplication::GetFileSystem()
#define ZGetDirectInput()        (&g_DInput)

#define ZGetQuest()                ((ZBaseQuest*)((ZApplication::GetGameInterface()) ? ZApplication::GetGameInterface()->GetQuest() : NULL))
#define ZGetQuestExactly()        ((ZQuest*)((ZApplication::GetGameInterface()) ? ZApplication::GetGameInterface()->GetQuestExactly() : NULL))
#define ZGetSurvivalExactly()    ((ZSurvival*)((ZApplication::GetGameInterface()) ? ZApplication::GetGameInterface()->GetSurvivalExactly() : NULL))

#define ZGetGameTypeManager()    ((ZApplication::GetGameInterface()) ? ZApplication::GetGameInterface()->GetGameTypeManager() : NULL)

#define ZGetInput()                (g_pInput)
#define ZGetCamera()            (ZApplication::GetGameInterface() ? ZApplication::GetGameInterface()->GetCamera() : NULL)

#define ZGetWorldManager()        ZApplication::GetInstance()->GetWorldManager()
#define ZGetWorld()                (ZGetWorldManager()->GetCurrent())

inline ZEmblemInterface*    ZGetEmblemInterface() { return ZApplication::GetInstance()->GetEmblemInterface(); }
inline ZOptionInterface*    ZGetOptionInterface(void) { return ZApplication::GetInstance()->GetOptionInterface(); }

#define ZIsActionKeyPressed(_ActionID)    (ZGetInput()->IsActionKeyPressed(_ActionID))

//jintriple3 메크로..
#define PROTECT_DEBUG_REGISTER(b) if(GetTickCount() >0)if(GetTickCount() >0)if(GetTickCount() >0)if(b)
//jintriple3 디버그 레지스터 해킹 방어 위한 비교 숫자.
#define FOR_DEBUG_REGISTER 1000
```

La decisión de forzar la inserción en línea (mediante macros) aumentó sustancialmente la cantidad de instrucciones generadas por el compilador.

## Sobreutilización de Accesores

En general, estos accesores, especialmente `ZGetGame`, están cómicamente sobreutilizados. Abra su proyecto preferido en un editor, vaya a `ZGame.cpp` y busque `ZGetGame`. En un mundo utópico, no vería ningún resultado. No vivimos en ese mundo y verá cientos de resultados. No hay ninguna razón, ninguna en absoluto, para llamar a `ZGetGame` dentro del ámbito de los métodos de `ZGame`.

"Pero generalmente `ZGetGame` comprueba nullptr, por lo que podría ser útil dentro del ámbito de `ZGame`" - **No.** Un puntero a una instancia de `ZGame` se almacena en `ecx` (x86) o `rcx` (x64) cada vez que se llama a un método de clase. Los métodos son un concepto de código. El compilador los trata como cualquier otra función, excepto que espera que un puntero a la instancia desde la que se llama al método esté en `ecx/rcx`. Además, `ZGetGame` devuelve `NULL` si la comprobación falla de todos modos.

Para que quede lo más claro posible:

```cpp
auto ZGame::SomeGameMethod() -> void {
    if (ZGetGame()->GetShaftCircumf() > 0x8008135) {
        ZGetGame()->m_pMyCharacter->OnDie();
    }
}
```

Debería escribirse como:

```cpp
auto ZGame::SomeGameMethod() -> void {
    if (GetShaftCircumf() > 0x8008135) {
        m_pMyCharacter->OnDie();
    }
}
```

(dentro del ámbito de `ZGame`)

Excepto que la segunda iteración no genera instrucciones excesivas ni ciclos de CPU desperdiciados.

## Recomendaciones

Cualquier método que invoque repetidamente `ZGet<Whatever>()`, es muy probable que se pueda y se deba invocar una vez, almacenar el resultado en una variable local y referenciarla en lugar de realizar más llamadas.

Este análisis es bastante pedante, lo sé. Pero mejorar es mejorar. No invoques repetidamente estos métodos de acceso; intenta eliminar las llamadas excesivas que ya existen. Si pensara que publicar una revisión completa de cómo se accede a estos datos en todo el código fuente sería útil, lo haría. Pero no creo que así sea.

## Estado Actual del Código

En el código fuente actual (`ZGlobal.cpp`), la implementación de `ZGetGame()` es:

```cpp
ZGame* ZGetGame(void) {
    return ZApplication::GetGameInterface() ? 
        ZApplication::GetGameInterface()->GetGame() : NULL;
}
```

Esta implementación mantiene las verificaciones de nullptr, pero sigue siendo susceptible a los problemas de rendimiento mencionados cuando se llama repetidamente dentro de funciones.

### Estadísticas

- **Usos de `ZGetGame()` en `ZGame.cpp`**: 9 ocurrencias directas
- **Usos totales en el proyecto**: 358 ocurrencias en 52 archivos
- **Problema principal**: Llamadas redundantes que no son optimizadas por el compilador
- **Impacto**: Overhead innecesario en rutas de código críticas

### Validación del Análisis

**✅ Este análisis ha sido validado contra el código fuente actual.** 

Para ver la validación completa con ejemplos específicos de problemas encontrados y soluciones recomendadas, consulta:
- **[VALIDACION_ANALISIS_SINGLETONS.md](./VALIDACION_ANALISIS_SINGLETONS.md)** - Análisis detallado de aplicabilidad al código actual

**Hallazgos principales de la validación:**
- El código actual usa funciones (no macros), lo cual es mejor que la versión 1.5
- Se encontraron casos específicos de uso problemático dentro de métodos de `ZGame`
- Algunas optimizaciones ya han sido aplicadas en partes del código
- El problema de sobreutilización es real y medible (358 usos en 52 archivos)

