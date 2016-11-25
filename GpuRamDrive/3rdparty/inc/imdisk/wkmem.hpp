#pragma once

inline void *operator_new(size_t Size, UCHAR FillByte)
{
    void * result = ExAllocatePoolWithTag(NonPagedPool, Size, POOL_TAG);

    if (result != NULL)
    {
        RtlFillMemory(result, Size, FillByte);
    }

    return result;
}

inline void * __CRTDECL operator new(size_t Size)
{
    return operator_new(Size, 0);
}

inline void * __CRTDECL operator new[](size_t Size)
{
    return operator_new(Size, 0);
}

inline void * __CRTDECL operator new(size_t Size, UCHAR FillByte)
{
    return operator_new(Size, FillByte);
}

inline void operator_delete(void *Ptr)
{
    if (Ptr != NULL)
    {
        ExFreePoolWithTag(Ptr, POOL_TAG);
    }
}

inline void __CRTDECL operator delete(void * Ptr)
{
    operator_delete(Ptr);
}

inline void __CRTDECL operator delete(void * Ptr, size_t)
{
    operator_delete(Ptr);
}

inline void __CRTDECL operator delete[](void * Ptr)
{
    operator_delete(Ptr);
}

template<typename T, POOL_TYPE pool_type> class WPoolMem
{
protected:
    T *ptr;
    SIZE_T bytecount;

    explicit WPoolMem(T *pBlk, SIZE_T AllocationSize)
        : ptr(pBlk),
        bytecount(pBlk != NULL ? AllocationSize : 0) { }

public:
    operator bool()
    {
        return ptr != NULL;
    }

    bool operator!()
    {
        return ptr == NULL;
    }

    operator T*()
    {
        return ptr;
    }

    T* operator ->()
    {
        return ptr;
    }

    T* operator+(int i)
    {
        return ptr + i;
    }

    T* operator-(int i)
    {
        return ptr - i;
    }

    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    SIZE_T Count() const
    {
        return GetSize() / sizeof(T);
    }

    SIZE_T GetSize() const
    {
        return ptr != NULL ? bytecount : 0;
    }

    void Free()
    {
        if (ptr != NULL)
        {
            ExFreePoolWithTag(ptr, POOL_TAG);
            ptr = NULL;
        }
    }

    void Clear()
    {
        if ((ptr != NULL) && (bytecount > 0))
        {
            RtlZeroMemory(ptr, bytecount);
        }
    }

    T* Abandon()
    {
        T* ab_ptr = ptr;
        ptr = NULL;
        bytecount = 0;
        return ab_ptr;
    }

    ~WPoolMem()
    {
        Free();
    }

    void Initialize(SIZE_T AllocateSize)
    {
        ptr = (T*)ExAllocatePoolWithTag(pool_type, AllocateSize, POOL_TAG);
        bytecount = AllocateSize;
    }

public:
    WPoolMem() :
        ptr(NULL),
        bytecount(0) { }

    explicit WPoolMem(SIZE_T AllocateSize)
    {
        Initialize(AllocateSize);
    }

    T* Alloc(SIZE_T AllocateSize)
    {
        Free();
        Initialize(AllocateSize);
        return ptr;
    }
};

class WHandle
{
private:
    HANDLE h;

public:
    operator bool()
    {
        return h != NULL;
    }

    bool operator !()
    {
        return h == NULL;
    }

    operator HANDLE()
    {
        return h;
    }

    void Close()
    {
        if (h != NULL)
        {
            ZwClose(h);
            h = NULL;
        }
    }

    WHandle() :
        h(NULL) { }

    explicit WHandle(HANDLE h) :
        h(h) { }

    ~WHandle()
    {
        Close();
    }
};

