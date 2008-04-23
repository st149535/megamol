/*
 * Interlocked.h
 *
 * Copyright (C) 2006 - 2007 by Universitaet Stuttgart (VIS). 
 * Alle Rechte vorbehalten.
 */

#ifndef VISLIB_INTERLOCKED_H_INCLUDED
#define VISLIB_INTERLOCKED_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif /* (defined(_MSC_VER) && (_MSC_VER > 1000)) */
#if defined(_WIN32) && defined(_MANAGED)
#pragma managed(push, off)
#endif /* defined(_WIN32) && defined(_MANAGED) */

#ifdef _WIN32
#include <windows.h>
#else /* _WIN32 */
//#include <asm/atomic.h>
#endif /* _WIN32 */

#include "vislib/UnsupportedOperationException.h"
#include "vislib/types.h"


namespace vislib {
namespace sys {


    /**
     * This class provides interlocked operations on both, Windows and Linux 
     * platforms.
     *
     * The interlocked functions perform an atomic operations on the destination
     * variables. They provide a simple mechanism for synchronising access to a
     * variable that is shared by multiple threads. The threads of different 
     * processes can use this mechanism if the variable is in shared memory.
     *
     * The parameters for interlocked functions must be aligned on a 32-bit 
     * boundary; otherwise, the functions will behave unpredictably on 
     * multiprocessor x86 systems and any non-x86 systems.
     *
     * Windows: These functions should not be used on memory allocated with the 
     * PAGE_NOCACHE modifier, as this may cause hardware faults on some 
     * processor architectures. To ensure ordering between reads and writer 
     * to PAGE_NOCACHE memory, use explicit memory barriers in your code.
     */
    class Interlocked {

    public:

        /** Dtor. */
        ~Interlocked(void);

        /**
         * Perform an atomic comparison of the specified 32-bit values and 
         * exchange them, based on the outcome of the comparison. If the 
         * variable at 'address' and 'comparand' are equal, the variable at
         * 'address' is exchanged with 'exchange'. Otherwise, nothing is node.
         *
         * @param address   Pointer to the destination value. The sign is 
         *                  ignored.
         * @param exchange  Exchange value. The sign is ignored. 
         * @param comparand Value to compare to 'address'. The sign is ignored.
         *
         * @return The initial value of the variable designated by 'address'.
         */
#ifdef _WIN32
        __forceinline static INT32 CompareExchange(volatile INT32 *address, 
                const INT32 exchange, const INT comparand) {
            return ::InterlockedCompareExchange(
                reinterpret_cast<volatile LONG *>(address), exchange, 
                comparand);
#else /* _WIN32 */
        inline static INT32 CompareExchange(volatile INT32 *address, 
                const INT32 exchange, const INT comparand) {
            INT32 retval;
            __asm__ __volatile__ ("lock; cmpxchgl %2, %0"
                : "=m" (*address), "=a" (retval)
                : "r" (exchange), "m" (*address), "a" (comparand)
                : "memory");
            return retval;
#endif /* _WIN32 */
        }

        /**
         * Decrement the value of the specified 32-bit variable designated by 
         * and return the resulting value.
         *
         * @param address Pointer to the variable to be decremented.
         *
         * @return The new value of the variable.
         */
#ifdef _WIN32
        __forceinline static INT32 Decrement(volatile INT32 *address) {
            return ::InterlockedDecrement(reinterpret_cast<volatile LONG *>(
                address));
#else /* _WIN32 */
        inline INT32 static Decrement(volatile INT32 *address) {
            return (Interlocked::ExchangeAdd(address, -1) - 1);
#endif /* _WIN32 */
        }

        /**
         * Atomically exchange a pair of 32-bit values.
         *
         * @param address Address of the target variable.
         * @param value   The new value of the variable.
         *
         * @return The old value of the variable designated by 'address'.
         */
#ifdef _WIN32
        __forceinline static INT32 Exchange(volatile INT32 *address, 
                const INT32 value) {
            return ::InterlockedExchange(reinterpret_cast<volatile LONG *>(
                address), value);
#else /* _WIN32 */
        inline static INT32 Exchange(volatile INT32 *address, 
                const INT32 value) {
            // TODO: This implementation is crazy.
            INT32 old, retval;

            old = *address;
            while (true) {
                retval = Interlocked::CompareExchange(address, value, old);
                if (retval == old) {
                    return retval;
                } else {
                    old = retval;
                }
            }
#endif /* _WIN32 */
        }

        /**
         * Perform an atomic addition of a 32-bit increment value to a 32-bit 
         * addend variable. 
         *
         * @param address Address of the addend variable.
         * @param value   Value to be added to the variable at 'address'.
         *
         * @return Value of variable designated by 'address' prior to the 
         *         operation.
         */
#ifdef _WIN32
        __forceinline static INT32 ExchangeAdd(volatile INT32 *address, 
                const INT32 value) {
            return ::InterlockedExchangeAdd(reinterpret_cast<volatile LONG *>(
                address), value);
#else /* _WIN32 */
        inline static INT32 ExchangeAdd(volatile INT32 *address, 
                const INT32 value) {
            INT32 retval;
            __asm__ __volatile__("lock; xaddl %0, %1"
                : "=r" (retval), "=m" (*address)
                : "0" (value), "m" (*address)
                : "memory");
            return retval;
#endif /* _WIN32 */
        }

        /**
         * Perform an atomic subtraction of a 32-bit increment value to a 32-bit
         * variable. 
         *
         * @param address Address of the variable to modify.
         * @param value   Value to be subtracted from the variable at 'address'.
         *
         * @return Value of variable designated by 'address' prior to the 
         *         operation.
         */
#ifdef _WIN32
        __forceinline static INT32 ExchangeSub(volatile INT32 *address, 
                const INT32 value) {
            return ::InterlockedExchangeAdd(reinterpret_cast<volatile LONG *>(
                address), -value);
#else /* _WIN32 */
        inline static INT32 ExchangeSub(volatile INT32 *address, 
                const INT32 value) {
            return Interlocked::ExchangeAdd(address, -value);
#endif /* _WIN32 */
        }

        /**
         * Increment the value of the specified 32-bit variable designated by 
         * and return the resulting value.
         *
         * @param address Pointer to the variable to be incremented.
         *
         * @return The new value of the variable.
         */
#ifdef _WIN32
        __forceinline static INT32 Increment(volatile INT32 *address) {
            return ::InterlockedIncrement(reinterpret_cast<volatile LONG *>(
                address));
#else /* _WIN32 */
        inline static INT32 Increment(volatile INT32 *address) {
            return (Interlocked::ExchangeAdd(address, 1) + 1);
#endif /* _WIN32 */
        }

    private:

        /**
         * Disallow instances. 
         *
         * @throws UnsupportedOperationException Unconditionally.
         */
        Interlocked(void);

    };
    
} /* end namespace sys */
} /* end namespace vislib */

#if defined(_WIN32) && defined(_MANAGED)
#pragma managed(pop)
#endif /* defined(_WIN32) && defined(_MANAGED) */
#endif /* VISLIB_INTERLOCKED_H_INCLUDED */
