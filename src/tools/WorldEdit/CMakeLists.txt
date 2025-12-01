# WorldEdit - MFC Application for World Editing
# Requiere MFC (Microsoft Foundation Classes)

if(NOT WIN32)
	message(WARNING "WorldEdit requires Windows and MFC, skipping...")
	return()
endif()

# CORRECCIÓN: MFC con _AFXDLL requiere /MD (runtime DLL compartida), no /MT (estático)
# Esto debe configurarse ANTES de crear el target
if(MSVC)
	# Configurar para usar DLL compartida (/MD) - REQUERIDO para MFC
	# Esto sobrescribe la configuración global de ucm_set_runtime(STATIC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Configurar MFC como DLL compartida (similar a Visual Studio)
set(CMAKE_MFC_FLAG 2)

# Recopilar todos los archivos fuente
file(GLOB src
	"*.cpp"
	"*.h"
)

# Recursos - archivos .rc y recursos en la carpeta res/
file(GLOB rc_files
	"*.rc"
)

file(GLOB res_files
	"res/*.rc2"
	"res/*.ico"
	"res/*.bmp"
)

# Verificar si existe targetver.h, si no, usar el de cml
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/targetver.h")
	# targetver.h se incluirá desde cml/Include
endif()

# Crear el ejecutable MFC con WIN32 para aplicación de ventana (no consola)
add_executable(WorldEdit WIN32 ${src} ${rc_files} ${res_files})

# Configurar propiedades MFC para Visual Studio
set_target_properties(WorldEdit PROPERTIES
	VS_MFC_FLAG 2  # Usar MFC como DLL compartida
	# Forzar runtime DLL compartida (/MD) - REQUERIDO para MFC con _AFXDLL
	MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

# Configurar subsistema Windows (aplicación GUI, no consola)
if(MSVC)
	set_target_properties(WorldEdit PROPERTIES
		LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	)
endif()

# Directorios de inclusión
target_include_directories(WorldEdit PRIVATE
	.
	../..
	../../RealSpace2/Include
	../../sdk/bullet/include
	../../sdk/dx9/include
	../../sdk
	../../cml/Include  # Para targetver.h si es necesario
)

# Definiciones del preprocesador
target_compile_definitions(WorldEdit PRIVATE
	_WIN32
	_WINDOWS
	_AFXDLL  # Usar MFC como DLL compartida
	_AFXEXT  # Extensiones MFC
	UNICODE
	_UNICODE
)

# Opciones del compilador para MFC
if(MSVC)
	# Configurar runtime como DLL compartida (/MD) - REQUERIDO para MFC con _AFXDLL
	target_compile_options(WorldEdit PRIVATE
		/utf-8
		/EHsc  # Manejo de excepciones C++
		/MP    # Compilación paralela
		/MD    # Runtime DLL compartida (requerido para MFC)
	)
	
	# Configurar también en el linker
	target_link_options(WorldEdit PRIVATE
		/SUBSYSTEM:WINDOWS
	)
endif()

# Enlazar librerías necesarias
target_link_libraries(WorldEdit PRIVATE
	RealSpace2
	# Bullet se enlaza a través de RealSpace2
	# MFC se enlaza automáticamente cuando CMAKE_MFC_FLAG está configurado
)

# Librerías del sistema Windows necesarias para MFC
if(MSVC)
	target_link_libraries(WorldEdit PRIVATE
		comctl32.lib
		comdlg32.lib
		shell32.lib
		ole32.lib
		oleaut32.lib
		uuid.lib
		winspool.lib
		advapi32.lib
		ws2_32.lib
		winmm.lib  # Para mmsystem.h usado en MainFrm.cpp
		d3dx9.lib  # Para funciones D3DX (D3DXMatrixInverse, D3DXVec3TransformCoord, etc.)
	)
endif()

# Configurar archivo de recursos (.rc) como archivo de recursos
if(rc_files)
	set_source_files_properties(${rc_files} PROPERTIES
		LANGUAGE RC
	)
endif()

# Instalar el ejecutable
install(TARGETS WorldEdit
	RUNTIME DESTINATION "tools/"
	COMPONENT tools
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

