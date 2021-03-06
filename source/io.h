#ifndef IO_HEADER
#define IO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 joyCfg;
extern u32 EMUinput;
extern u32 g_subBatteryLevel;
extern u32 batteryLevel;

/**
 * Copies the time from the NDS RTC to the NGP RTC.
 */
void transferTime(void);

/**
 * Reads a byte from the other system. If no data is available or no
 * high-level communications have been established, then return FALSE.
 * If buffer is NULL, then no data is read, only status is returned
 */
bool system_comms_read(char *buffer);

/**
 * Peeks at any data from the other system. If no data is available or
 * no high-level communications have been established, then return FALSE.
 * If buffer is NULL, then no data is read, only status is returned
 */
bool system_comms_poll(char *buffer);

/**
 * Writes a byte from the other system. This function should block until
 * the data is written. USE RELIABLE COMMS! Data cannot be re-requested.
 */
void system_comms_write(char data);

/**
 * Reads the "appropriate" (system specific) flash data into the given
 * preallocated buffer. The emulation core doesn't care where from.
 */
bool system_io_flash_read(char *buffer, int bufferLength);

/**
 * Writes the given flash data into an "appropriate" (system specific)
 * place. The emulation core doesn't care where to.
 */
bool system_io_flash_write(char *buffer, int bufferLength);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// IO_HEADER
