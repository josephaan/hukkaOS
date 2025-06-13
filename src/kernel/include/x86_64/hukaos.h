#ifndef __HUKAOS__H
#define __HUKAOS__H

#include <stdint.h>

typedef struct{
    char osname[6];
    uint32_t version;
    uint32_t arch;

    // CPU Information
    uint32_t CPU_Family;
    uint32_t CPU_Model;
    uint32_t CPU_Stepping;
    char CPU_vendor[13];
    char CPU_brand[49];
    uint32_t CPU_extended_model;
    uint32_t CPU_extended_family;
    char CPU_Features[6][12];
    char CPU_Extended_Features[10][12];

    //

} os_information_t __attribute__((packed));

#endif // !__HUKAOS__H