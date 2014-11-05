#ifndef _UTIL_H_
#define _UTIL_H_
#define SAFE_RELEASE(p) \
	if(p) \
	p->Release();\
	p = NULL;
#endif