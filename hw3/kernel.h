#ifndef _H_KERNEL
#define _H_KERNEL

typedef struct _systemframe {
    int id;

    int status;
    void* address;
} Systemframe;

void init_system_page_frame();
int allocate_page();
void free_page(int page_frame_id);
void* get_pa_from_page_frame(int page_frame_id);
void swap_and_load_frame(Systemframe *frame);
void save_to_disk_frame(Systemframe *frame);
void load_from_disk_frame(Systemframe *frame);

#endif