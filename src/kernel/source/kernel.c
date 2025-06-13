#include <kernel.h>


/* imported virtual addresses, see linker script */
extern BOOTBOOT bootboot;               // see bootboot.h
extern unsigned char environment[4096]; // configuration, UTF-8 text key=value pairs
extern uint8_t fb;                      // linear framebuffer mapped

// In your process creation or context switch code
void* user_stack_start = (void*)0x00007FFFFFFFF000; // Allocate a stack for the user process

void _start() {

    	
    serial_init();
    idt_init();
    init_gdt();
    physical_memory_init();
    parse_acpi();
    apic_init();
    initialize_framebuffer_details();
    ioapic_init();
    init_apic_timer_interrupt();
    enable_keyboard();
    init_user_program_memory();
        

    /*** NOTE: this code runs on all cores in parallel ***/
    int x, y, s=bootboot.fb_scanline, w=bootboot.fb_width, h=bootboot.fb_height;

    if(s) {
        // cross-hair to see screen dimension detected correctly
        for(y=0;y<h;y++) { *((uint32_t*)(&fb + s*y + (w*2)))=0x00FFFFFF; }
        for(x=0;x<w;x++) { *((uint32_t*)(&fb + s*(h/2)+x*4))=0x00FFFFFF; }
        
        uint32_t current_core_id = get_current_core_id();
        
        if (current_core_id == bootboot.bspid) {

            // red, green, blue boxes in order
            for(y=0;y<20;y++) { for(x=0+300;x<20+300;x++) { *((uint32_t*)(&fb + s*(y+20) + (x+20)*4))=0x00FF0000; } }
            for(y=0;y<20;y++) { for(x=0+300;x<20+300;x++) { *((uint32_t*)(&fb + s*(y+20) + (x+50)*4))=0x0000FF00; } }
            for(y=0;y<20;y++) { for(x=0+300;x<20+300;x++) { *((uint32_t*)(&fb + s*(y+20) + (x+80)*4))=0x000000FF; } }
            //draw_hukaos_logo();
            
            disp_str("HukaOS kernel");
            print_numstr("\nby Joseph Haita (c) 2024\n\n", 0xff345e);

            //detect_cpu();
            identify_cpu();
            ata_init();

                     
            //draw_kyle();
                
            print_sdt_type();
            parse_memory_map();

            print_numstr("\nThe base address of rsp  is: %p", Cyan, get_rsp());

            
            
            /* ... */
            uint64_t vaddr_start = (uint64_t)kernel_virtual_start;  
            uint64_t vaddr_end = (uint64_t)kernel_virtual_end;  
            print_numstr("\nThe base address of kernel start: %p", Cyan, vaddr_start);
            print_numstr("\nThe base address of kernel end: %p", Cyan, vaddr_end);
            //print_numstr("\nThe base address of tsc frequency  is: %lu", Cyan, estimate_tsc_freq());

            //create_window("Haita Joseph", 480, 600);
            //draw_kyle();
            //draw_hukaos_logo();
            //create_window("Snake Game", 480, 600);

            //init_gui();
            
            
            //run_snake_game();
            

           //keep_top_pml4_entry();

            new_paging();

            //switch_to_user_mode(user_stack_start); // Pass the top of the stack
           
            terminal_write("\nHello World %p", &bootboot);


            log(LOG_LEVEL_INFO, "After Page Switch");
            
            disp_str("\n\nHello Again");

        }

    }
    // hang for now
    while(1);
}


