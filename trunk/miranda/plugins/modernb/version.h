#define BUILD_NUM 102
#define BUILD_NUM_STR  "102"
#define REVISION  "$Revision$"

#define COREVERSION_NUM 0, 8, 0,
#define COREVERSION_NUM_STR  "0, 8, 0 "

#define MINIMAL_COREVERSION_NUM PLUGIN_MAKE_VERSION(0, 8, 0, 9)
#define MINIMAL_COREVERSION 0, 8, 0, 9

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
