include_directories(../Shared)

# Do not build collada converter for ios or android
if( (NOT ${CMAKE_SYSTEM_NAME} MATCHES "iOS") AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android") AND (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten"))
add_executable(ColladaConv 
	converter.h
	daeCommon.h
	daeLibAnimations.h
	daeLibControllers.h
	daeLibEffects.h
	daeLibGeometries.h
	daeLibImages.h
	daeLibMaterials.h
	daeLibVisualScenes.h
	daeMain.h
	optimizer.h
	utils.h
	converter.cpp
	daeMain.cpp
	main.cpp
	optimizer.cpp
	utils.cpp
	)
endif()
