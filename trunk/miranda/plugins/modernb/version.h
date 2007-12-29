#define BUILD_NUM 35
#define BUILD_NUM_STR  "35"
#define REVISION  "$Revision$"

#ifdef FORCE_HOTKEY_IN_MODERN
	#define COREVERSION_NUM 0, 7, 100,
	#define COREVERSION_NUM_STR  "0, 7, 100 "
#else
	#define COREVERSION_NUM 0, 8, 0,
	#define COREVERSION_NUM_STR  "0, 8, 0 "
#endif //FORCE_HOTKEY_IN_MODERN


#define MINIMAL_COREVERSION_NUM PLUGIN_MAKE_VERSION(0, 7, 0, 34)
#define MINIMAL_COREVERSION 0, 7, 0, 34

#ifdef UNICODE
#define VER_UNICODE " UNICODE"
#else
#define VER_UNICODE " ANSI"
#endif

#ifdef DEBUG
#define VER_DEBUG " DEBUG"
#else
#define VER_DEBUG " "
#endif

#define FILE_VERSION	COREVERSION_NUM BUILD_NUM
#define FILE_VERSION_STR COREVERSION_NUM_STR  VER_DEBUG VER_UNICODE " build " BUILD_NUM_STR	" " REVISION 

#define PRODUCT_VERSION	FILE_VERSION
#define PRODUCT_VERSION_STR	FILE_VERSION_STR
