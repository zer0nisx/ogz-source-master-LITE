# MaterialTest - Proyecto de Prueba para Materiales Iluminados

## 📋 Descripción

Este proyecto es una aplicación de prueba independiente basada en RealSpace2 para experimentar y validar shaders de materiales iluminados. Permite probar diferentes configuraciones de materiales, luces y propiedades de iluminación sin necesidad de ejecutar el juego completo.

## 🎯 Objetivo

- Probar shaders de materiales iluminados de forma aislada
- Validar diferentes configuraciones de materiales (mate, brillante, plástico, etc.)
- Experimentar con múltiples luces puntuales
- Verificar cálculos de iluminación difusa y especular
- Probar diferentes modelos de iluminación (Blinn-Phong, Phong, etc.)

## 📁 Estructura del Proyecto

```
MaterialTest/
├── CMakeLists.txt          # Configuración de CMake
├── main.cpp                # Punto de entrada de la aplicación
├── MaterialTestApp.h       # Declaración de la clase principal
├── MaterialTestApp.cpp     # Implementación de la aplicación
├── stdafx.h                # Precompilados
├── targetver.h             # Versión de Windows
├── Shaders/
│   └── MaterialTest.hlsl   # Shader de prueba para materiales iluminados
└── MATERIAL_SHADER_REQUIREMENTS.md  # Documentación de requisitos del shader
```

## 🔧 Compilación

### Requisitos Previos

1. **DirectX SDK (June 2010)** instalado
2. **CMake** 3.5 o superior
3. **Visual Studio** con soporte para C++14
4. **RealSpace2** compilado como biblioteca estática

### Pasos de Compilación

1. **Compilar RealSpace2 primero:**
   ```bash
   cd build
   cmake ..
   cmake --build . --target RealSpace2
   ```

2. **Compilar MaterialTest:**
   ```bash
   cmake --build . --target MaterialTest
   ```

3. **Compilar el shader HLSL:**
   ```bash
   cd src/RealSpace2
   # Editar BuildHLSLShaders.bat para agregar MaterialTest
   # O compilar manualmente:
   fxc /T vs_3_0 /FhInclude/MaterialTest.h /VnMaterialTestData Source/MaterialTest.hlsl /O3
   ```

## 🚀 Ejecución

1. Asegúrate de que el ejecutable esté en el directorio correcto con acceso a:
   - Los shaders compilados
   - Cualquier recurso necesario (texturas, meshes, etc.)

2. Ejecuta `MaterialTest.exe`

3. **Controles:**
   - `ESC`: Salir de la aplicación
   - El objeto debería rotar automáticamente para mostrar la iluminación desde diferentes ángulos

## 📊 Qué Probar

### Materiales Diferentes

Modifica `MaterialTestApp::SetupMaterial()` para probar:

- **Material Mate**: `MaterialSpecular = (0,0,0,0)`, `MaterialPower = 0`
- **Material Brillante (Metal)**: `MaterialSpecular = (1,1,1,1)`, `MaterialPower = 32`
- **Material Plástico**: `MaterialSpecular = (0.8,0.8,0.8,1)`, `MaterialPower = 16`
- **Material Goma**: `MaterialSpecular = (0.2,0.2,0.2,1)`, `MaterialPower = 4`

### Luces Diferentes

Modifica `MaterialTestApp::SetupLights()` para probar:

- Diferentes colores de luz
- Diferentes posiciones
- Diferentes rangos
- Diferentes atenuaciones

### Shader Modifications

Edita `Shaders/MaterialTest.hlsl` para:

- Probar diferentes modelos de iluminación
- Agregar nuevas características
- Optimizar cálculos
- Experimentar con diferentes técnicas

## 📚 Documentación

Consulta `MATERIAL_SHADER_REQUIREMENTS.md` para información detallada sobre:

- Qué componentes son necesarios en el shader
- Cómo funcionan los cálculos de iluminación
- Qué registros se usan y para qué
- Ejemplos de diferentes materiales
- Mejores prácticas

## 🔍 Debugging

- Los logs se escriben en `MaterialTest.log`
- Usa `MLog()` para agregar mensajes de debug
- Verifica que los shaders se compilen correctamente
- Asegúrate de que las constantes del shader se establezcan correctamente

## 🎨 Próximos Pasos

- [ ] Agregar soporte para cargar meshes desde archivo
- [ ] Agregar interfaz para cambiar materiales en tiempo real
- [ ] Agregar soporte para texturas
- [ ] Agregar más tipos de luces (directional, spot)
- [ ] Agregar visualización de normales y vectores de luz
- [ ] Agregar exportación de capturas de pantalla

## 📝 Notas

- Este proyecto usa el mismo sistema de shaders que el juego principal
- Los shaders deben compilarse antes de ejecutar la aplicación
- El proyecto está configurado para usar DirectX 9 (Shader Model 3.0)
- Para usar Vulkan, se necesitarían modificaciones adicionales

