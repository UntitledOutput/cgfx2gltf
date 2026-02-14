#include "lz11.h"
#include <stdlib.h>
#include <string.h>

/**
 * LZ11 decompression implementation for Nintendo 3DS format
 * 
 * LZ11 Format:
 * - Header: 1 byte (0x11) + 3 bytes (decompressed size in little-endian)
 * - Compressed data follows with flag bytes and literal/reference pairs
 */

int lz11_is_compressed(const uint8_t *data, size_t size) {
    if (size < 4) {
        return 0;
    }
    
    // Check for LZ11 magic byte (0x11)
    return data[0] == 0x11;
}

uint8_t *lz11_decompress(const uint8_t *compressed, size_t compressed_size, size_t *decompressed_size) {
    if (compressed_size < 4 || compressed[0] != 0x11) {
        return NULL;
    }
    
    // Read decompressed size from header (3 bytes, little-endian)
    size_t output_size = compressed[1] | (compressed[2] << 8) | (compressed[3] << 16);
    
    // If decompressed size is 0, it's stored as 4 additional bytes
    size_t input_offset = 4;
    if (output_size == 0) {
        if (compressed_size < 8) {
            return NULL;
        }
        output_size = compressed[4] | (compressed[5] << 8) | (compressed[6] << 16) | (compressed[7] << 24);
        input_offset = 8;
    }
    
    uint8_t *output = malloc(output_size);
    if (!output) {
        return NULL;
    }
    
    size_t output_offset = 0;
    
    while (output_offset < output_size && input_offset < compressed_size) {
        uint8_t flag_byte = compressed[input_offset++];
        
        for (int i = 0; i < 8 && output_offset < output_size; i++) {
            if (flag_byte & 0x80) {
                // Compressed block
                if (input_offset >= compressed_size) {
                    free(output);
                    return NULL;
                }
                
                uint8_t byte1 = compressed[input_offset++];
                uint8_t indicator = (byte1 >> 4);
                
                size_t length;
                size_t displacement;
                
                if (indicator == 0) {
                    // Type 0: 3 bytes - AB CD 0E
                    // Length is (((AB & 0x0F) << 4) | (CD >> 4)) + 0x11
                    // Displacement is (((CD & 0x0F) << 8) | 0E) + 1
                    if (input_offset + 1 >= compressed_size) {
                        free(output);
                        return NULL;
                    }
                    uint8_t byte2 = compressed[input_offset++];
                    uint8_t byte3 = compressed[input_offset++];
                    
                    length = (((byte1 & 0x0F) << 4) | (byte2 >> 4)) + 0x11;
                    displacement = (((byte2 & 0x0F) << 8) | byte3) + 1;
                    
                } else if (indicator == 1) {
                    // Type 1: 4 bytes - 1B CD EF GH
                    // Length is ((((BC) << 4) | (DE >> 4)) << 4 | (EF & 0x0F)) + 0x111
                    // Displacement is (((EF & 0x0F) << 8) | GH) + 1
                    if (input_offset + 2 >= compressed_size) {
                        free(output);
                        return NULL;
                    }
                    uint8_t byte2 = compressed[input_offset++];
                    uint8_t byte3 = compressed[input_offset++];
                    uint8_t byte4 = compressed[input_offset++];
                    
                    length = ((((byte1 & 0x0F) << 12) | (byte2 << 4) | (byte3 >> 4))) + 0x111;
                    displacement = (((byte3 & 0x0F) << 8) | byte4) + 1;
                    
                } else {
                    // Type 2: 2 bytes - AB CD
                    // Length is (AB >> 4) + 1
                    // Displacement is (((AB & 0x0F) << 8) | CD) + 1
                    if (input_offset >= compressed_size) {
                        free(output);
                        return NULL;
                    }
                    uint8_t byte2 = compressed[input_offset++];
                    
                    length = indicator + 1;
                    displacement = (((byte1 & 0x0F) << 8) | byte2) + 1;
                }
                
                // Copy from earlier in output buffer
                if (displacement > output_offset) {
                    free(output);
                    return NULL;
                }
                
                size_t copy_pos = output_offset - displacement;
                for (size_t j = 0; j < length && output_offset < output_size; j++) {
                    output[output_offset++] = output[copy_pos++];
                }
                
            } else {
                // Uncompressed byte (literal)
                if (input_offset >= compressed_size) {
                    free(output);
                    return NULL;
                }
                output[output_offset++] = compressed[input_offset++];
            }
            
            flag_byte <<= 1;
        }
    }
    
    if (decompressed_size) {
        *decompressed_size = output_size;
    }
    
    return output;
}
