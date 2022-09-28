#ifndef NGPFLASH_HEADER
#define NGPFLASH_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BLOCKS (35)

extern u32 flashSize;

/**
 * Check if a flash block is modifed compared to original file.
 * @param  chip: Which chip to check, 0 or 1
 * @param  block: Which block to check, 0 to 34
 * @retval True if block is modified.
 */
bool isBlockDirty(int chip, int block);

/**
 * Mark a flash block as modified compared to original file.
 * @param  chip: Which chip to mark, 0 or 1.
 * @param  block: Which block to mark, 0 to 34.
 * @retval True if block could be marked as modified.
 */
bool markBlockDirty(int chip, int block);

/**
 * Get the offset of a block in the flash chip.
 * @param  block: Block nr, 0 to 34.
 * @retval Offset of block in bytes in flash chip memory.
 */
int getBlockOffset(int block);

/**
 * Get the size of a block in the flash chip.
 * @param  block: Block nr, 0 to 34.
 * @retval Size of block in bytes in flash chip memory.
 */
int getBlockSize(int block);

/**
 * Get block number from an address/offset in the flash chip.
 * @param  address: The address to convert to block number.
 * @retval The block corresponding to the specified address.
 */
int getBlockFromAddress(int address);


void *getFlashLOBlocksAddress(void);
void *getFlashHIBlocksAddress(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NGPFLASH_HEADER
