#include "dm.h"

void *
memory_grow_to_size(void *mem_p,
                    size_t *mem_cap_ip,
                    size_t mem_size_i)
{
    /* null memory block */
    if(!mem_p) {
        *mem_cap_ip = mem_size_i;
        return malloc(mem_size_i);
    }
    
    /* if need more memory then double cap until we have it */
    if(mem_size_i > *mem_cap_ip) {
        while(mem_size_i > *mem_cap_ip) {
            *mem_cap_ip *= 2;
        }
        
        mem_p = realloc(mem_p, *mem_cap_ip);
    }
    
    return mem_p;
}
