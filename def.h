#ifndef _DEF_H_
#define _DEF_H_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::ostream;
#include <algorithm>
#include <assert.h>
using std::min;
using std::max;
using std::swap;
using std::sort;


#define Assert(expr) ((void)0)


#define RAY_EPSILON 1e-3f

#define sqrtf sqrt
//class Vector;
//class Point;
//class Normal;
//class Ray;

inline float Radians(float deg) {
     return ((float)M_PI/180.f) * deg;
}

class  ReferenceCounted {
public:
	ReferenceCounted() { nReferences = 0; }
	int nReferences;
private:
	ReferenceCounted(const ReferenceCounted &);
	ReferenceCounted &operator=(const ReferenceCounted &);
};

template <class T> class Reference {
public:
	// Reference Public Methods
	Reference(T *p = NULL) {
		ptr = p;
		if (ptr) ++ptr->nReferences;
	}
	Reference(const Reference<T> &r) {
		ptr = r.ptr;
		if (ptr) ++ptr->nReferences;
	}
	Reference &operator=(const Reference<T> &r) {
		if (r.ptr) r.ptr->nReferences++;
		if (ptr && --ptr->nReferences == 0) delete ptr;
		ptr = r.ptr;
		return *this;
	}
	Reference &operator=(T *p) {
		if (p) p->nReferences++;
		if (ptr && --ptr->nReferences == 0) delete ptr;
		ptr = p;
		return *this;
	}
	~Reference() {
		if (ptr && --ptr->nReferences == 0)
			delete ptr;
	}
	T *operator->() { return ptr; }
	const T *operator->() const { return ptr; }
	operator bool() const { return ptr != NULL; }
	bool operator<(const Reference<T> &t2) const {
		return ptr < t2.ptr;
	}
private:
	T *ptr;
};

#endif
