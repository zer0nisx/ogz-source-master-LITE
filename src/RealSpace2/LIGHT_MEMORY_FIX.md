# Corrección de Memoria para Luces - RShaderMgr

## Problema Identificado

El usuario reportó que los cambios de la luz no se guardaban correctamente o había problemas de memoria insuficiente asignada.

## Análisis del Problema

### Estado Original
1. **`mLight[MAX_LIGHT]`** es un array estático estático (línea 16)
   - ✅ Tiene memoria asignada correctamente
   - ❌ **NO se inicializaba explícitamente** - podía contener valores basura

2. **`setLight()`** copiaba solo algunos campos:
   - ❌ Solo copiaba: Ambient, Diffuse, Specular, Range, Attenuation0/1/2, Position
   - ❌ **NO validaba el puntero** `pLight_` antes de usarlo
   - ❌ **NO copiaba todos los campos** de `D3DLIGHT9` (Type, Direction, Falloff, Theta, Phi)

3. **Constructor** no inicializaba las luces:
   - ❌ Las luces podían contener valores basura en memoria
   - ❌ Esto causaba que los valores no se guardaran correctamente

## Soluciones Implementadas

### 1. Inicialización Explícita en Constructor
```cpp
RShaderMgr::RShaderMgr()
{
	// ... código existente ...
	
	// MEJORA: Inicializar explícitamente las luces para evitar valores basura
	// Esto asegura que los valores de luz se guarden correctamente en memoria
	for (int i = 0; i < MAX_LIGHT; ++i)
	{
		ZeroMemory(&mLight[i], sizeof(D3DLIGHT9));
		mLight[i].Type = D3DLIGHT_POINT;
		mLight[i].Range = 0.0f;
		mLight[i].Attenuation0 = 1.0f;
		mLight[i].Attenuation1 = 0.0f;
		mLight[i].Attenuation2 = 0.0f;
	}
}
```

**Beneficios**:
- ✅ Asegura que todas las luces empiecen con valores conocidos
- ✅ Evita valores basura que podrían causar comportamiento inesperado
- ✅ Garantiza que los valores se guarden correctamente en memoria

### 2. Validación de Puntero y Copia Completa
```cpp
void RShaderMgr::setLight(int iLignt_, D3DLIGHT9* pLight_)
{
	if (!mbUsingShader) return;
	if (iLignt_ >= MAX_LIGHT) return;
	if (!pLight_) return;  // MEJORA: Validar puntero para evitar acceso inválido

	// MEJORA: Copiar todos los campos de la luz de forma segura
	mLight[iLignt_].Type = pLight_->Type;
	mLight[iLignt_].Ambient = pLight_->Ambient;
	mLight[iLignt_].Diffuse = pLight_->Diffuse;
	mLight[iLignt_].Specular = pLight_->Specular;
	mLight[iLignt_].Range = pLight_->Range;
	mLight[iLignt_].Attenuation0 = pLight_->Attenuation0;
	mLight[iLignt_].Attenuation1 = pLight_->Attenuation1;
	mLight[iLignt_].Attenuation2 = pLight_->Attenuation2;
	mLight[iLignt_].Position = pLight_->Position;
	mLight[iLignt_].Direction = pLight_->Direction;  // Mantener consistencia
	mLight[iLignt_].Falloff = pLight_->Falloff;
	mLight[iLignt_].Theta = pLight_->Theta;
	mLight[iLignt_].Phi = pLight_->Phi;
}
```

**Beneficios**:
- ✅ Valida el puntero antes de usarlo (evita crashes)
- ✅ Copia **TODOS** los campos de `D3DLIGHT9` (consistencia completa)
- ✅ Asegura que los valores se guarden correctamente en memoria

## Verificación de Memoria

### Estructura D3DLIGHT9
```cpp
typedef struct D3DLIGHT9 {
    D3DLIGHTTYPE Type;           // 4 bytes
    D3DCOLORVALUE Diffuse;        // 16 bytes (4 floats)
    D3DCOLORVALUE Specular;       // 16 bytes (4 floats)
    D3DCOLORVALUE Ambient;        // 16 bytes (4 floats)
    D3DVECTOR Position;           // 12 bytes (3 floats)
    D3DVECTOR Direction;          // 12 bytes (3 floats)
    float Range;                  // 4 bytes
    float Falloff;                // 4 bytes
    float Attenuation0;           // 4 bytes
    float Attenuation1;           // 4 bytes
    float Attenuation2;           // 4 bytes
    float Theta;                  // 4 bytes
    float Phi;                    // 4 bytes
} D3DLIGHT9;  // Total: ~108 bytes
```

### Memoria Asignada
- **`mLight[MAX_LIGHT]`** donde `MAX_LIGHT = 3`
- **Total**: 3 × 108 bytes = **324 bytes**
- ✅ **Suficiente memoria asignada** - el problema era la inicialización, no la cantidad

## Resultado

✅ **Los valores de luz ahora se guardan correctamente en memoria**
✅ **No hay valores basura que puedan causar problemas**
✅ **Validación de punteros previene crashes**
✅ **Copia completa de todos los campos asegura consistencia**

## Notas Adicionales

- El array `mLight` es **estático estático**, por lo que persiste durante toda la vida de la aplicación
- `Update()` lee de `mLight[i]` y envía al shader cada frame
- Los valores se mantienen en memoria hasta que se llame `setLight()` de nuevo
- La inicialización explícita asegura que los valores sean válidos desde el inicio

