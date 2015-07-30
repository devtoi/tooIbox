file(GLOB_RECURSE UtilitySources
	"Logger.h"
	"Logger.cpp"
	"Config.h"
	"Config.cpp"
	"CalculateWorldMatrix.h"
	"CalculateWorldMatrix.cpp"
	"CallbackConfig.h"
	"CallbackConfig.cpp"
	"CallbackRegister.h"
	"ConfigManager.h"
	"ConfigManager.cpp"
	"FileUtility.h"
	"FileUtility.cpp"
	"ConsoleColors.h"
	"UtilityLibraryDefine.h"
	"GameTimer.h"
	"GameTimer.cpp"
	"Randomizer.h"
	"IncrementableHandle.h"
	"Handle.h"
	"HandleAllocator.h"
	"PlatformDefinitions.h"
	"RetardedWindowsIncludes.h"
	"SerializationUtility.h"
	"SerializationUtility.cpp"
	"DataSizes.h"
	"ThreadingUtility.h"
	"SetUtility.h"
	"Byte.h"
	"BitfFlagUtility.h"
	"Vec4Colors.h"
	"LocklessQueue.h"
)

set(UTILITY_MEMORY_LIB)
if(USE_CUSTOM_ALLOCATOR_HEADER_FOR_UTILITY)
	set(UTILITY_MEMORY_LIB Memory)
	add_definitions(-DUTILITY_ALLOCATION_HEADER=${CUSTOM_ALLOCATOR_HEADER_FOR_UTILITY})
else(USE_CUSTOM_ALLOCATOR_HEADER_FOR_UTILITY)
	list(APPEND UtilitySources "Alloc.h")
	add_definitions(-DUTILITY_ALLOCATION_HEADER="Alloc.h")
endif(USE_CUSTOM_ALLOCATOR_HEADER_FOR_UTILITY)

add_definitions(-DUTILITY_DLL_EXPORT)
# Create library for utilities
add_library(Utility SHARED ${UtilitySources})
target_link_libraries(Utility Memory ${standardLibraries} ${SDL2Library})

install(
	TARGETS Utility DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin/${OperatingSystemNameLowerCase}/${TargetArchitecture})
