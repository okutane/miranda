#define BUILD_NUM 9
#define BUILD_NUM_STR  "9"

#define COREVERSION_NUM 0, 9, 0, 
#define COREVERSION_NUM_STR  "0, 9, 0 "

#define MINIMAL_COREVERSION_NUM PLUGIN_MAKE_VERSION(0, 9, 0, 0)
#define MINIMAL_COREVERSION 0, 9, 0, 0
#define MINIMAL_COREVERSION_STR "0, 9, 0, 0 "

#ifdef UNICODE
#define VER_UNICODE " UNICODE"
#else
#define VER_UNICODE " ANSI"
#endif

#ifdef UNICODE
#define UNICODE_AWARE_STR "(UNICODE)"
#else
#define UNICODE_AWARE_STR "(ANSI)"
#endif

#ifdef DEBUG
#define DEBUG_AWARE_STR "Debug "
#else
#define DEBUG_AWARE_STR ""
#endif

#define FILE_VERSION	COREVERSION_NUM BUILD_NUM
#define FILE_VERSION_STR COREVERSION_NUM_STR  DEBUG_AWARE_STR " " UNICODE_AWARE_STR " build " BUILD_NUM_STR	" " 

#define PRODUCT_VERSION	FILE_VERSION
#define PRODUCT_VERSION_STR	FILE_VERSION_STR
