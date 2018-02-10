#pragma once
#include <algorithm>
#include <guiddef.h>


template<typename T> class ComPtr
{
public:
    typedef typename T InterfaceType;
protected:
    T* m_ptr;
    template<class U> friend class ComPtr;
public:
    // Shamelessly copied from the WRL ComPtr
    // Helper object that makes IUnknown methods private
    template <typename U>
    class RemoveIUnknown : public U
    {
    private:
        // STDMETHOD macro implies virtual.
        // ComPtr can be used with any class that implements the 3 methods of IUnknown.
        // ComPtr does not require these methods to be virtual.
        // When ComPtr is used with a class without a virtual table, marking the functions
        // as virtual in this class adds unnecessary overhead.
        long __stdcall QueryInterface( REFIID riid, void **ppvObject );
        unsigned long __stdcall AddRef();
        unsigned long __stdcall Release();
    };

    ComPtr( void ) noexcept : ComPtr( nullptr ) {};

    ComPtr( std::nullptr_t ) noexcept : m_ptr(nullptr) {};

    ComPtr( T* const other ) noexcept : m_ptr( other )
    {
        InternalAddRef();
    }

    ComPtr( ComPtr<T> const & other ) noexcept : ComPtr( other.m_ptr ) {}

    ComPtr( ComPtr<T> &&other ) noexcept : ComPtr()
    {
        swap( *this, other );
    }

    /// copy constructor that allows to instantiate class when U* is convertible to T*
    template<typename U, typename = typename std::enable_if_t<std::is_convertible<U*, T*>::value> >
    ComPtr( const ComPtr<U> &other ) noexcept : ComPtr( other.m_ptr ) {}

    template<class U, typename = typename std::enable_if_t<std::is_convertible<U*, T*>::value> >
    ComPtr( ComPtr<U> &&other ) noexcept : ComPtr()
    { 
       swap(m_ptr, other.m_ptr);
    }


    ~ComPtr( void ) noexcept
    {
        InternalRelease();
    }

    ComPtr<T>& operator&( ) = delete;

    friend inline void swap( ComPtr<T>& first, ComPtr<T>& second ) noexcept
    {
        using std::swap;
        swap( first.m_ptr, second.m_ptr );
    }

    friend inline bool equal( ComPtr<T> const & first, ComPtr<T> const & second ) noexcept
    {
        return ( first.m_ptr == second.m_ptr );
    }

    ComPtr<T>& operator=( ComPtr<T> other ) noexcept
    {
        swap( *this, other );
        return *this;
    }

    T* Get() const noexcept
    {
        return m_ptr;
    }

    T* const* GetAddressOf() const noexcept
    {
        return &m_ptr;
    }

    T** GetAddressOf() noexcept
    {
        return &m_ptr;
    }

    T** ReleaseAndGetAddressOf() noexcept
    {
        InternalRelease();
        return &m_ptr;
    }

    unsigned long Reset() noexcept
    {
        return InternalRelease();
    }

    explicit operator bool() const noexcept
    {
        return !empty();
    }

    bool empty() const noexcept
    {
        return (m_ptr == nullptr);
    }

    operator T*( ) const noexcept
    {
        return m_ptr;
    }

    typename RemoveIUnknown<T>* operator->( ) const noexcept
    {
        return static_cast<typename RemoveIUnknown<T>*>( m_ptr );
    }
protected:
    void InternalAddRef() const noexcept
    {
        if( !empty() )
        {
            m_ptr->AddRef();
        }
    }

    unsigned long InternalRelease() noexcept
    {
        auto result = empty() ? 0 : m_ptr->Release();
        m_ptr = nullptr;
        return result;
    }
};
