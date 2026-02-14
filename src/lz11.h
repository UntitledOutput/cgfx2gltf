#ifndef LZ11_H
#define LZ11_H

#include <stdint.h>
#include <stddef.h>

/**
 * Check if data is LZ11 compressed by looking for the magic byte
 * @param data Pointer to the data buffer
 * @param size Size of the data buffer
 * @return 1 if data is LZ11 compressed, 0 otherwise
 */
int lz11_is_compressed(const uint8_t *data, size_t size);

/**
 * Decompress LZ11 compressed data
 * @param compressed Pointer to compressed data
 * @param compressed_size Size of compressed data
 * @param decompressed_size Pointer to store the decompressed size (output)
 * @return Pointer to decompressed data (caller must free), or NULL on error
 */
uint8_t *lz11_decompress(const uint8_t *compressed, size_t compressed_size, size_t *decompressed_size);

#endif // LZ11_H
