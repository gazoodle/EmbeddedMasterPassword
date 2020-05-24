///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  romix.h - Header file for ROMix implementation
//
//  Copyright (C) 2020, Gazoodle (https://github.com/gazoodle)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _inc_romix_h
#define _inc_romix_h

#include <stdint.h>
#include <functional>
#include "salsa20.h"

/*
    Algorithm scryptROMix https://tools.ietf.org/html/rfc7914#page-6

        Input:
            r       Block size parameter.
            B       Input octet vector of length 128 * r octets.
            N       CPU/Memory cost parameter, must be larger than 1,
                    a power of 2, and less than 2^(128 * r / 8).

        Output:
            B'      Output octet vector of length 128 * r octets.

        Steps:

            1.  X = B

            2.  for i = 0 to N - 1 do
                    V[i] = X
                    X = scryptBlockMix (X)
                end for

            3.  for i = 0 to N - 1 do
                    j = Integerify (X) mod N
                        where Integerify (B[0] ... B[2 * r - 1]) is defined
                        as the result of interpreting B[2 * r - 1] as a
                        little-endian integer.
                    T = X xor V[j]
                    X = scryptBlockMix (T)
                end for

            4. B' = X
*/            

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ## Comments and thoughts ##
//
//  The ROMix routine is where the scrypt algorithm gets its memory hardness from. From the RFC
//  you can choose your parameters to determine the effective hardness that this algorithm would
//  experience when running on hardware limited by memory or by processor speed. The MPW algorithm
//  chooses to drive it with N=32768, R=8 and P=2 which leads to a need for a 32MB buffer for ROMix
//  to perform it's loop rollout. On most modern devices, this isn't too much of a problem, but
//  on any MCU this is way out of the range of available memory.
//
//  It turns out that you can trade memory for performance in the algorithm by repeating the ROMix
//  rounds on demand at the expense of not storing every V array entry. At the worst case, we could 
//  do this rollout loop for every requirement (i.e. have no pre-cached values and compute them on
//  demand in the second phase of ROMix), but with the above constants, that results in an unfeasable 
//  amount of time repeating the rollout.
//
//  At best, we could have the full N entries of the pre-computed V array to work with (or 32MB). 
//
//  What we really need however is something in between, a sparse set of entries arranged such that 
//  we can start with the closest previous calculated V array entry and then only have to work from 
//  that to the required mix depth.
//
//  The sparseness of the precomputed V array can be anything from 0 (i.e. every entry is present)
//  through to N (where no entries are present). At 0, the algorithm is at its fastest, and at
//  N its slowest. Where we actually are on this spectrum depends entirely on how much memory we
//  can get access to.
//
//  On some MCU boards, the available memory is split between heap and stack and while techinically
//  possible to trade between these, a much nicer solution is to support both, so not only is the 
//  sparse implementation capable of handling various sizes of buffer, that buffer can be split
//  in several ways. 
//
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::function<void (uint8_t percent)> progress_func;
#define min(a,b)        ((a)<(b)?(a):(b))
#define max(a,b)        ((a)>(b)?(a):(b))

template<uint32_t N, uint32_t r, uint32_t p, uint32_t dkLen, uint32_t stack_allocation, uint32_t heap_allocation>
class scrypt_mixer
{
public:
    scrypt_mixer(void) : scrypt_mixer(0,0) {}
    scrypt_mixer(Salsa20Block* global_buffer, uint32_t global_size) : m_heap_buffer(0), m_global_buffer(global_buffer), sparse_factor(1)
    {
        sparse_v_malloc_blocks = heap_allocation / ( r * 2 * sizeof(Salsa20Block) );
        sparse_v_stack_blocks = countof(m_stack_buffer)/(r*2);
        sparse_v_global_blocks = global_size / ( r * 2 * sizeof(Salsa20Block) );
        if ( sparse_v_malloc_blocks + sparse_v_stack_blocks + sparse_v_global_blocks > 0 )
            sparse_factor = min( N, max(1, ( N / (sparse_v_malloc_blocks + sparse_v_stack_blocks + sparse_v_global_blocks ))));
        if ( N % (sparse_v_malloc_blocks + sparse_v_stack_blocks + sparse_v_global_blocks ) != 0 )
            sparse_factor++;
        if ( sparse_v_malloc_blocks > 0 )
            m_heap_buffer = (Salsa20Block*)malloc(r*2*sparse_v_malloc_blocks*sizeof(Salsa20Block));
        //Serial << "N=" << N << " malloc_blocks=" << sparse_v_malloc_blocks << " stack_blocks=" << sparse_v_stack_blocks << " global_blocks=" << sparse_v_global_blocks << " sparse_factor=" << sparse_factor << endl;
    }
    ~scrypt_mixer(void)
    {
        if ( m_heap_buffer != 0 )
        {
            free(m_heap_buffer);
            m_heap_buffer = 0;
        }
    }

    inline void Salsa20(Salsa20Block& block, uint8_t rounds) const
    {
        /*
            Implement algorithm from https://tools.ietf.org/html/rfc7914#page-4
        */
        Salsa20Block X(block);
        for (uint8_t i = 0; i < rounds; i += 2)
            X.DoubleRound();
        block.Add(X);
    }

    inline void BlockMix(const Salsa20Block *input, Salsa20Block *output) const
    {
        /*
            Implement algorithmn from https://tools.ietf.org/html/rfc7914#page-5


            Parameters:
                r       Block size parameter.

            Input:
                B[0] || B[1] || ... || B[2 * r - 1]
                    Input octet string (of size 128 * r octets),
                    treated as 2 * r 64-octet blocks,
                    where each element in B is a 64-octet block.

            Output:
                B'[0] || B'[1] || ... || B'[2 * r - 1]
                    Output octet string.

            Steps:

                1.  X = B[2 * r - 1]

                2.  for i = 0 to 2 * r - 1 do
                        T = X xor B[i]
                        X = Salsa (T)
                        Y[i] = X
                    end for

                3. B' = (Y[0], Y[2], ..., Y[2 * r - 2],
                        Y[1], Y[3], ..., Y[2 * r - 1])        
        */

        // 1.  X = B[2 * r - 1]
        Salsa20Block X(input[(2*r)-1]);
        // 2.  for i = 0 to 2 * r - 1 do
        for (unsigned int i = 0; i < (2*r); i++)
        {
            // T = X xor B[i]
            X.Xor(input[i]);
            // X = Salsa (T)
            Salsa20(X, 8);

            // 3. B' = (Y[0], Y[2], ..., Y[2 * r - 2],
            //          Y[1], Y[3], ..., Y[2 * r - 1])
            memcpy(&output[( r * (i&1)) + (i>>1)], &X, sizeof(X));
        }
    }

    inline Salsa20Block* get_v_ptr(uint32_t index)
    {
        // Convert to sparse index
        index = index/sparse_factor;
        if ( sizeof(m_stack_buffer) > 0 )
        {
            if ( index < sparse_v_stack_blocks )
                return &m_stack_buffer[r*2*index];
            index -= sparse_v_stack_blocks;
        }
        if ( index < sparse_v_malloc_blocks )
            return &m_heap_buffer[r*2*index];
        index -= sparse_v_malloc_blocks;
        return &m_global_buffer[r*2*index];
    }

    void ROMix(Salsa20Block* block, progress_func progress)
    {
        if (progress)(progress)(0);

        if ( sparse_v_malloc_blocks > 0 && ( m_heap_buffer == 0 ))
        {
            /*
            #ifdef ARDUINO
            Serial << "Oops" << endl;
            #else
            printf("Oops\n");
            #endif
            return;
            */
        }

        // 1. X = B
        memcpy( X, block, sizeof(X));

        // 2. Build V array
        for(uint32_t i=0; i < N; i++)
        {
            // V[i] = X
            if (i%sparse_factor==0)
                memcpy(get_v_ptr(i), X, sizeof(X));
            // X = scryptBlockMix (X)
            BlockMix(X, T);
            memcpy(X, T, sizeof(X));
        }

        if (progress)(progress)(5);

        // 3. Perform integerify mix loop
        for (uint32_t i = 0; i < N; i++)
        {
            // j = Integerify (X) mod N
            //          where Integerify (B[0] ... B[2 * r - 1]) is defined
            //          as the result of interpreting B[2 * r - 1] as a
            //          little-endian integer.
            uint32_t j = X[(r*2)-1].entry[0].as_word32 % N;
            // T = X xor V[j]
            memcpy( LocalV, get_v_ptr(j), sizeof(LocalV) );
            for (uint32_t k = (j/sparse_factor)*sparse_factor; k < j; k++)
            {
                BlockMix(LocalV, T);
                memcpy(LocalV, T, sizeof(LocalV));
            }

            for (uint32_t k = 0; k < r*2 ; k++ )
                T[k].Xor(X[k], LocalV[k]);
            // X = scryptBlockMix (T)
            BlockMix(T, X);

            if (progress)(progress)( 5 + ( i * 95 / N ));
        }

        // 4. X = B'
        memcpy( block, X, sizeof(X));
    }

    void Mix(Salsa20Block* block, progress_func progress)
    {
        for(uint32_t i=0; i<p; i++, block += r*2)
            ROMix(block, [&] ( uint8_t percent ) {
                if (progress)(progress)( ( i * 100 / p ) + ( percent / p ) );
            });
    }

private:
    Salsa20Block    m_stack_buffer[stack_allocation/sizeof(Salsa20Block)];
    Salsa20Block*   m_heap_buffer;
    Salsa20Block*   m_global_buffer;
    Salsa20Block    X[r*2];
    Salsa20Block    T[r*2];
    Salsa20Block    LocalV[r*2];
    uint32_t        sparse_v_global_blocks;
    uint32_t        sparse_v_malloc_blocks;
    uint32_t        sparse_v_stack_blocks;
    uint32_t        sparse_factor;
};

#endif
