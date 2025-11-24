cmake_policy(SET CMP0015 NEW)

# Proyecto de prueba para materiales iluminados
project(MaterialTest)

# Archivos fuente
set(SOURCES
    main.cpp
    MaterialTestApp.cpp
)

# Crear ejecutable
add_executable(MaterialTest ${SOURCES})

# Incluir directorios
target_include_directories(MaterialTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cml/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../RealSpace/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk
)

# Enlazar con RealSpace2 y dependencias
target_link_libraries(MaterialTest PRIVATE
    RealSpace2
    cml
    rapidxml
)

# Configuración específica de Windows/MSVC
if (MSVC)
    target_include_directories(MaterialTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/dx9/include)
    find_library(dxerr_path NAMES dxerr PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/dx9/lib/x86)
    if (dxerr_path)
        target_link_libraries(MaterialTest PRIVATE ${dxerr_path})
    endif()
    
    # Flags del compilador
    target_compile_options(MaterialTest PRIVATE
        /Zi
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -D_USE_GDIPLUS
    )
    
    # Configurar subsistema Windows y punto de entrada WinMain
    set_target_properties(MaterialTest PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup"
    )
    
    # Agregar biblioteca para resolver __vsnprintf en DxErr.lib
    target_link_libraries(MaterialTest PRIVATE legacy_stdio_definitions.lib)
    
    # Nota: Los shaders se compilan manualmente usando BuildHLSLShaders.bat
    # o usando fxc directamente. El shader se puede compilar con:
    # fxc /T vs_3_0 /FhInclude/MaterialTest.h /VnMaterialTestData Source/MaterialTest.hlsl /O3
endif()

