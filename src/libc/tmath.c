/*
 * math.c
 * 
 *
 * Copyright (C) 2024 - 2026 Joseph Haita (haitajoseph2@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the HukkaOS. 
 * 
 *
 */

#include <stdint.h>
#include <tmath.h>

 uint64_t uint64_power(uint64_t base, uint64_t expo){

    if(expo == 0){
        return 1;
    }
    else {
        long ans = base;
        for (int i = 0; (uint64_t)i < (expo - 1) ; i++){
            base *= ans;
        }
    }

	return base;
    
}

// Returns absolute of a given value
int32_t absolute(int32_t val){
    if (val < 0) return -val;
    else return val;
}


// uint32_t float_to_uint32(float_t val){ return (uint32_t)val; }

// There is potential for incorrect results due to floating point rounding error
// float_t round(float_t val, uint32_t dec_place) {     return val; }