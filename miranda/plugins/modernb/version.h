// 0, 7, 17, 4


#define BUILD_NUM 4
#define BUILD_NUM_STR  "4"

#define MINIMAL_COREVERSION_NUM PLUGIN_MAKE_VERSION(0, 7, 0, 17)
#define MINIMAL_COREVERSION 0, 7, 0, 17

#define COREVERSION_NUM 0, 7, 17,
#define COREVERSION_NUM_STR  "0, 7, 17 "

#define FILE_VERSION	COREVERSION_NUM BUILD_NUM
#define PRODUCT_VERSION	COREVERSION_NUM BUILD_NUM

#ifdef UNICODE
#ifndef _DEBUG
#define FILE_VERSION_STR	COREVERSION_NUM_STR " UNICODE build " BUILD_NUM_STR
	#define PRODUCT_VERSION_STR	COREVERSION_NUM_STR " UNICODE build " BUILD_NUM_STR 
#else
#define FILE_VERSION_STR	COREVERSION_NUM_STR " DEBUG UNICODE build " BUILD_NUM_STR
#define PRODUCT_VERSION_STR	COREVERSION_NUM_STR " DEBUG UNICODE build " BUILD_NUM_STR
#endif
#else
#ifndef _DEBUG
#define FILE_VERSION_STR	COREVERSION_NUM_STR " ANSI build " BUILD_NUM_STR
#define PRODUCT_VERSION_STR	COREVERSION_NUM_STR " ANSI build " BUILD_NUM_STR 
#else
#define FILE_VERSION_STR	COREVERSION_NUM_STR " DEBUG ANSI build " BUILD_NUM_STR
#define PRODUCT_VERSION_STR	COREVERSION_NUM_STR " DEBUG ANSI build " BUILD_NUM_STR
#endif

#endif
