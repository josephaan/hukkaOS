/*
*
*
* This header file include syscall function prototypes and definitions as well as structures for HUKAOS
* Created by Joseph Haita (c) 2024 
*/
#ifndef __HUKAOS__H
#define __HUKAOS__H

#include <stdint.h>

uint64_t syscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);


#endif // !__HUKAOS__H
