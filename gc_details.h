
template <class T>
class PtrDetails
{
  public:
    unsigned int refcount; 
    T *memPtr;        
    bool isArray;
    unsigned int arraySize; 
    PtrDetails(T *p, long long s) : memPtr(p), arraySize(s)
    {
        if(arraySize>0)
            isArray = true;
        else
            isArray = false;
    }
    template <class U>
    friend bool operator==(const PtrDetails<T> &ob1, const PtrDetails<T> &ob2);
};

template <class T>
bool operator==(const PtrDetails<T> &ob1, const PtrDetails<T> &ob2){
    if((ob1.memPtr == ob2.memPtr)&&(ob1.refcount == ob2.refcount))
        return true;
    return false;
}