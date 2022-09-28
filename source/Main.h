#ifndef MAIN_HEADER
#define MAIN_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern bool gameInserted;
extern u16 *menuMap;

void waitVBlank(void);

/**
 * Waits the specified number of frames before returning.
 * @param  count: Number of frames to wait.
 * @deprecated Don't use, solve it some other way.
 */
void pausVBlank(int count);

void setEmuSpeed(int speed);
void setupMenuPalette(void);
void setupBorderPalette(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MAIN_HEADER
