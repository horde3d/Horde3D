include_directories(../Shared)

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

