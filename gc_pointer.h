#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"
/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > refContainer;
    T *addr;
    bool isArray; 
    unsigned arraySize; 
    static bool first;
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);
public:
    typedef Iter<T> GCiterator;
    
    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);
    Pointer(const Pointer &);
    ~Pointer();
    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();
    // Overload assignment of pointer to Pointer.
    Pointer &operator=(T *t);
    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);
    // Return a reference to the object pointed
    // to by this Pointer.
    T &operator*(){
        return *addr;
    }
    // Return the address being pointed to.
    T *operator->() { return addr; }
    // Return a reference to the object at the
    // index specified by i.
    T &operator[](int i){ return addr[i];}
    // Conversion function to T *.
    operator T *() { return addr; }
    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }
    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }
    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }
    // A utility function that displays refContainer.
    static void showlist();
    // Clear refContainer when program exits.
    static void shutdown();
};


template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T *t){
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;
    PtrDetails<T> ob(t, size);
    this->addr = ob.memPtr;
    this->arraySize = ob.arraySize;
    this->isArray = ob.isArray;
    typename std::list<PtrDetails<T> >::iterator pt;
    bool isEqualTo = false;
    for (pt = refContainer.begin(); pt != refContainer.end(); pt++){
        if(ob.memPtr == pt->memPtr){
            ++(pt->refcount);
            isEqualTo = true;
            break;
        }
    }
    if(!isEqualTo){
        ob.refcount = 1;
        refContainer.push_back(ob);
    }
}
// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer<T, size> &ob){
    this->addr = ob.addr;
    this->arraySize = ob.arraySize;
    this->isArray = ob.isArray;
    typename std::list<PtrDetails<T>>::iterator pt;
    for(pt = refContainer.begin(); pt!=refContainer.end(); pt++){
        if(ob.addr == pt->memPtr){
            ++(pt->refcount);
            break;
        }
    }
}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){
    typename std::list<PtrDetails<T>>::iterator p;
    p = findPtrInfo(addr);
    --(p->refcount);
    collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){
    bool memfreed;
    typename std::list<PtrDetails<T>>::iterator p;
    do{
      for(p = refContainer.begin(); p!=refContainer.end(); p++){
        if(p->refcount == 0){
            if(p->memPtr != nullptr){
                if(p->isArray)
                    delete[] p->memPtr;
                else
                    delete p->memPtr;
            }
            refContainer.remove(*p);
            memfreed = true;
            break;
        }
      }
    }while(p != refContainer.end());
    return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(T *t){
    if (first)
        atexit(shutdown);
    first = false;
    typename std::list<PtrDetails<T>>::iterator p;
    p = findPtrInfo(addr);
    if(p != refContainer.end())
        --(p->refcount);

    p = findPtrInfo(t);
    if(p != refContainer.end()) {
        ++(p->refcount);
        addr = t;
        isArray = p->isArray;
        arraySize = p->arraySize;
        return *this;
    }
    else {
        PtrDetails<T> ptr(t, size);
        addr = ptr.memPtr;
        arraySize = ptr.arraySize;
        isArray = ptr.isArray;
        ptr.refcount = 1;
        refContainer.push_back(ptr);
        return *this;
    }
}

// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer<T, size> &rv){
    typename std::list<PtrDetails<T>>::iterator p;
    p = findPtrInfo(addr);
    --(p->refcount);
    p = findPtrInfo(rv.addr);
    ++(p->refcount);
    addr = p->memPtr;
    isArray = p->isArray;
    arraySize = p->arraySize;
    return *this;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}