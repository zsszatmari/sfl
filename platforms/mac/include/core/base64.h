//
//  base64.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/26/13.
//
//

#ifndef G_Ear_Player_base64_h
#define G_Ear_Player_base64_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "Environment.h"


core_export char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);
core_export unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *length);
                             
void build_decoding_table();
void base64_cleanup();
    
#ifdef __cplusplus
}
#endif
    
#endif
