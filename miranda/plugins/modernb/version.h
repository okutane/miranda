// 0, 4, 3, 66
#define BUILD_NUM 66
#define BUILD_NUM_STR  "66"

#define FILE_VERSION	0, 4, 3, BUILD_NUM
#define PRODUCT_VERSION	0, 4, 3, BUILD_NUM

#ifdef UNICODE
#ifndef _DEBUG
#define FILE_VERSION_STR	"0, 4, 3, UNICODE build " BUILD_NUM_STR
	#define PRODUCT_VERSION_STR	"0, 4, 3, UNICODE build " BUILD_NUM_STR 
#else
#define FILE_VERSION_STR	"0, 4, 3, DEBUG UNICODE build " BUILD_NUM_STR
#define PRODUCT_VERSION_STR	"0, 4, 3, DEBUG UNICODE build " BUILD_NUM_STR
#endif
#else
#ifndef _DEBUG
	#define FILE_VERSION_STR	"0, 4, 3, build " BUILD_NUM_STR
	#define PRODUCT_VERSION_STR	"0, 4, 3, build " BUILD_NUM_STR
#else
	#define FILE_VERSION_STR	"0, 4, 3, DEBUG build " BUILD_NUM_STR
	#define PRODUCT_VERSION_STR	"0, 4, 3, DEBUG build " BUILD_NUM_STR
#endif
#endif
