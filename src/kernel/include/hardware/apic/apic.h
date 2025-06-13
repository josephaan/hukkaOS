#ifndef __APIC__H
#define __APIC__H

#include <stdbool.h>
#include <stdint.h>

// Local APIC definitions
//#define[ Register name]                 [Offset] 	// [Read/Write permissions]

#define APIC_RESERVED_00                   0x000 
#define APIC_RESERVED_000                  0x010	
#define LAPIC_ID_REGISTER                  0x020 	  // Read/Write
#define LAPIC_VERSION_REGISTER             0x030     // Read only
#define	APIC_RESERVED_01                   0x040    // 040h-070h 	
#define	TASK_PRIORITY_REGISTER             0x080   // Read/Write
#define	ARBITRATION_PRIORITY_REGISTER      0x090  // Read only
#define PROCESSOR_PRIORITY_REGISTER        0x0A0 // Read only
#define	EOI_REGISTER                       0x0B0 	// Write only
#define REMOTE_READ_REGISTER               0x0C0 	// Read  only
#define LOGICAL_DESTINATION_REGISTER       0x0D0 	// Read/Write
#define DESTINATION_FORMAT_REGISTER        0x0E0 	// Read/Write
#define SPURIOUS_INTERRUPT_VECTOR_REGISTER 0x0F0 	// Read/Write
#define IN_SERVICE_REGISTER                0x100    // 100h - 170h Read only
#define TRIGGER_MODE_REGISTER              0x180 	// 180h - 1F0h Read only
#define INTERRUPT_REQUEST_REGISTER         0x200    // 200h - 270h Read only
#define ERROR_STATUS_REGISTER              0x280 	// Read only
#define RESERVED_02                        0x290  // 290h - 2E0h  	
#define LVT_CMCI_REGISTER 	               0x2F0  // LVT_Corrected_Machine_Check_Interrupt, Read/Write
#define INTERRUPT_COMMAND_REGISTER	       0x300  // 300h - 310h, Read/Write
#define	LVT_TIMER_REGISTER                 0x320          // Read/Write
#define	LVT_THERMAL_SENSOR_REGISTER 	   0x330          // Read/Write
#define	LVT_PERFOMANCE_MONITORING_COUNTERS_REGISTER 0x340 // Read/Write
#define	LVT_LINT0_REGISTER                          0x350 // Read/Write
#define	LVT_LINT1_REGISTER                          0x360 // Read/Write
#define	LVT_ERROR_REGISTER 	                        0x370 // Read/Write
#define	INITIAL_COUNT_REGISTER_FOR_TIMER            0x380 // Read/Write
#define CURRENT_COUNT_REGISTER_FOR_TIMER            0x390 // Read only
#define	RESERVED_03	                                0x3A0 // 3A0h - 3D0h
#define DIVIDE_CONFIGURATION_REGISTER_FOR_TIMER     0x3E0 // Read/Write
#define	RESERVED_04                                 0x3F0 //>

#define IA32_APIC_BASE_MSR          0x1B
#define IA32_APIC_BASE_MSR_ENABLE   0x800
#define APIC_BASE_MSR_ENABLE        (1<<11)
#define APIC_TIMER_FLAG_PERIODIC    (1 << 17)
#define APIC_TIMER_FLAG_MASKED      (1 << 16)

typedef enum {
    APIC_TIMER_MODE_PERIODIC,
    APIC_TIMER_MODE_ONESHOT
} apic_timer_mode_t;

// Local APIC function prototypes
void write_apic_register(uint32_t reg, uint32_t value) ;
uint32_t read_apic_register(uint32_t reg);
void send_eoi() ;
void apic_init();
bool is_apic_enabled_in_msr();
void remap_pic();


// APIC Timer prototypes

void init_apic_timer_interrupt();
void apic_timer_interrupt_handler();
void sleep(uint64_t miliseconds);


#endif // ! __APIC__H