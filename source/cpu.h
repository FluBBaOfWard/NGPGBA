#ifndef CPU_HEADER
#define CPU_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 waitMaskIn;
extern u8 waitMaskOut;

void run(void);
void stepFrame(void);
void cpuReset(void);
bool isConsoleRunning(void);
bool isConsoleSleeping(void);

/**
 * Change emulated cpu speed between normal or half for speedup on low power devices.
 * @param hack: True to lower speed, false to reset to normal speed.
 */
void tweakCpuSpeed(bool hack);

/**
 * Change z80 cpu speed, can make emulation less power demanding.
 * @param downShift: How many times the speed is shifted down.
 */
void tweakZ80Speed(int downShift);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CPU_HEADER
