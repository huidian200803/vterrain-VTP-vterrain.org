//
// ScopedPtr.h
//

#pragma once

/**
   Mimics a built-in pointer, except that it guarantees deletion of the object
   pointed to.  A semi-compatible subset of boost's scoped_ptr.
 */
template<class T> class scoped_ptr
{
public:
    explicit scoped_ptr(T * p = 0) : px(p) { }
    ~scoped_ptr()			{ delete px; }
    T & operator*() const	{ return *px; }
    T * operator->() const	{ return px; }
    void set(T * p = 0)		{ px = p;    }
	T * get() const			{ return px; }
	operator T *() const	{ return px; }

private:
    T * px;
};

