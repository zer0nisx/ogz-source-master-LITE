# WorldEdit - ImGui Application for World Editing
# Usa ImGui del SDK en lugar de MFC

if(NOT WIN32)
	message(WARNING "WorldEdit requires Windows, skipping...")
	return()
endif()

# Recopilar todos los archivos fuente (excluyendo archivos MFC específicos)
file(GLOB src
	"*.cpp"
	"*.h"
)

# Excluir archivos MFC específicos que serán reemplazados
list(REMOVE_ITEM src 
	"${CMAKE_CURRENT_SOURCE_DIR}/WorldEdit.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/WorldEditView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/MainFrm.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/WorldEditDoc.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ControlDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ObjectDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PathDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/InfoDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ProgressDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GenerateLightmapDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GeneratePathDataDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LightmapDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ObjectSelectDialog.cpp"
)

# Agregar nuevos archivos sin MFC
list(APPEND src 
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/WorldEditView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/WorldEditDoc.cpp"
)

# Crear el ejecutable Win32 (no MFC)
add_executable(WorldEdit WIN32 ${src})

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
	../../sdk/imgui  # ImGui del SDK
	../../sdk
	../../cml/Include  # Para targetver.h si es necesario
)

# Definiciones del preprocesador
target_compile_definitions(WorldEdit PRIVATE
	_WIN32
	_WINDOWS
	UNICODE
	_UNICODE
)

# Opciones del compilador
if(MSVC)
	target_compile_options(WorldEdit PRIVATE
		/utf-8
		/EHsc  # Manejo de excepciones C++
		/MP    # Compilación paralela
	)
	
	# Configurar también en el linker
	target_link_options(WorldEdit PRIVATE
		/SUBSYSTEM:WINDOWS
	)
endif()

# Enlazar librerías necesarias
target_link_libraries(WorldEdit PRIVATE
	RealSpace2
	imgui  # ImGui del SDK
	# Bullet se enlaza a través de RealSpace2
)

# Librerías del sistema Windows necesarias
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
		winmm.lib
		d3d9.lib
		dinput8.lib
		dxguid.lib
	)
endif()

# Instalar el ejecutable
install(TARGETS WorldEdit
	RUNTIME DESTINATION "tools/"
	COMPONENT tools
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

