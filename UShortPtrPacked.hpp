#ifndef USHORT_PTR_PACKED_HPP
#define USHORT_PTR_PACKED_HPP

#include <bit>
#include <cstdint>

#ifdef _MSC_VER

#include <intrin.h>

#else

#include <cpuid.h>
#include <sys/mman.h>

#endif

template<uint8_t valueBits = 16>
class UShortPtrPacked
{
    static_assert(valueBits == 7 or valueBits == 16, "Value must be either 7 or 16 bits in length.");

    static constexpr uintptr_t PTR_MASK { (valueBits == 7) ? 0x01FFFFFFFFFFFFFFULL : 0x0000FFFFFFFFFFFFULL };

    static constexpr uint64_t TAG_MASK { (valueBits == 7) ? 127ULL : 65535ULL };

    static constexpr int SHIFT { (valueBits == 7) ? 57 : 48 };

    static constexpr int SIGN_EXT_SHIFT { 64 - SHIFT };

    void* ptr { nullptr };

public:

    UShortPtrPacked() = default;

    UShortPtrPacked(const UShortPtrPacked&) = default;

    UShortPtrPacked(UShortPtrPacked&&) = default;

    UShortPtrPacked& operator=(const UShortPtrPacked&) = default;

    UShortPtrPacked& operator=(UShortPtrPacked&&) = default;

    UShortPtrPacked(const unsigned short i, void* const p) noexcept
    {
        this->setUnsignedShortAndPtr(i, p);
    }

    unsigned short getUnsignedShort() const noexcept
    {
        return static_cast<unsigned short>((reinterpret_cast<uintptr_t>(this->ptr) >> UShortPtrPacked::SHIFT) bitand UShortPtrPacked::TAG_MASK);
    }

    template<class T>
    T* getCastedPtr() const noexcept
    {
        intptr_t signed_ptr = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(this->ptr));
        
        return reinterpret_cast<T*>((signed_ptr << UShortPtrPacked::SIGN_EXT_SHIFT) >> UShortPtrPacked::SIGN_EXT_SHIFT);
    }

    template<class T>
    T getFuncCastedPtr() const noexcept
    {
        return std::bit_cast<T>(this->getCastedPtr<void>());
    }

    void setUnsignedShort(const unsigned short i) noexcept
    {
        const uintptr_t raw_ptr { reinterpret_cast<uintptr_t>(this->ptr) bitand UShortPtrPacked::PTR_MASK };

        const uintptr_t tag { (static_cast<uintptr_t>(i) bitand UShortPtrPacked::TAG_MASK) << UShortPtrPacked::SHIFT };

        this->ptr = reinterpret_cast<void*>(raw_ptr bitor tag);
    }

    void setPtr(void* const p) noexcept
    {
        const uintptr_t current_tag { reinterpret_cast<uintptr_t>(this->ptr) bitand compl UShortPtrPacked::PTR_MASK };

        const uintptr_t raw_ptr { reinterpret_cast<uintptr_t>(p) bitand UShortPtrPacked::PTR_MASK };

        this->ptr = reinterpret_cast<void*>(raw_ptr bitor current_tag);
    }

    template<class T>
    void setFuncPtr(T* const p) noexcept
    {
        this->setPtr(std::bit_cast<void*>(p));
    }

    void setUnsignedShortAndPtr(const unsigned short i, void* const p) noexcept
    {
        const uintptr_t raw_ptr { reinterpret_cast<uintptr_t>(p) bitand UShortPtrPacked::PTR_MASK };

        const uintptr_t tag { (static_cast<uintptr_t>(i) bitand UShortPtrPacked::TAG_MASK) << UShortPtrPacked::SHIFT };

        this->ptr = reinterpret_cast<void*>(raw_ptr bitor tag);
    }

    template<class T>
    void setUnsignedShortAndFuncPtr(const unsigned short i, T* const p) noexcept
    {
        const uintptr_t raw_ptr { reinterpret_cast<uintptr_t>(std::bit_cast<void*>(p)) bitand UShortPtrPacked::PTR_MASK };

        const uintptr_t tag { (static_cast<uintptr_t>(i) bitand UShortPtrPacked::TAG_MASK) << UShortPtrPacked::SHIFT };

        this->ptr = reinterpret_cast<void*>(raw_ptr bitor tag);
    }

    static bool isOSProviding57BitAddressing() noexcept
    {
        int cpuInfo[4] = { 0 };

        #ifdef _MSC_VER

        __cpuidex(cpuInfo, 7, 0);

        #else

        __cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);

        #endif

        if ( const bool cpu_has_la57 { (cpuInfo[2] >> 16) bitand 1 }; not cpu_has_la57 ) 
            return false;

        void* hint { reinterpret_cast<void*>(0x1000000000000ULL) };

        const size_t page_size { 4096 };

        void* ptr { mmap(hint, page_size, PROT_READ bitor PROT_WRITE, MAP_PRIVATE bitor MAP_ANONYMOUS, -1, 0) };

        if ( ptr == MAP_FAILED )
            return false;

        const bool success { (reinterpret_cast<uintptr_t>(ptr) >= 0x1000000000000ULL) };

        munmap(ptr, page_size);

        return success;
    }
};

#endif
