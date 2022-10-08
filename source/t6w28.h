/*
*/

#ifndef T6W28_HEADER
#define T6W28_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u16 ch0Frq;
	u16 ch0Cnt;
	u16 ch1Frq;
	u16 ch1Cnt;
	u16 ch2Frq;
	u16 ch2Cnt;
	u16 ch3Frq;
	u16 ch3Cnt;

	u32 rng;
	u32 noiseFB;

	u8 ch0Vol;
	u8 ch1Vol;
	u8 ch2Vol;
	u8 ch3Vol;

	u8 ch0VolL;
	u8 ch1VolL;
	u8 ch2VolL;
	u8 ch3VolL;

	u8 snLastReg;
	u8 snLastRegL;
	u8 snPadding[2];

	u16 ch0Reg;
	u16 ch0Att;
	u16 ch1Reg;
	u16 ch1Att;
	u16 ch2Reg;
	u16 ch2Att;
	u16 ch3Reg;
	u16 ch3Att;

	u16 ch0RegL;
	u16 ch0AttL;
	u16 ch1RegL;
	u16 ch1AttL;
	u16 ch2RegL;
	u16 ch2AttL;
	u16 ch3RegL;
	u16 ch3AttL;

	u32 mixLength;
	u32 mixRate;
	u32 freqConv;
	u32 freqTablePtr;

} T6W28;

/**
 * Reset/initialize T6W28 chip.
 * @param  *chip: The T6W28 chip.
 */
void t6W28Reset(T6W28 *chip);

/**
 * Saves the state of the T6W28 chip to the destination.
 * @param  *destination: Where to save the state.
 * @param  *chip: The T6W28 chip to save.
 * @return The size of the state.
 */
int t6W28SaveState(void *destination, const T6W28 *chip);

/**
 * Loads the state of the T6W28 chip from the source.
 * @param  *chip: The T6W28 chip to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int t6W28LoadState(T6W28 *chip, const void *source);

/**
 * Gets the state size of a T6W28.
 * @return The size of the state.
 */
int t6W28GetStateSize(void);

/**
 * Runs the sound chip for len number of cycles, output is 1/4 samples,
 * so if actual chip would output 218kHz this mixer would render at ~55kHz.
 * @param  *len: Number of cycles to run.
 * @param  *dest: Pointer to buffer where sound is rendered.
 * @param  *chip: The SN76496 chip.
 */
void t6W28Mixer(int len, void *dest, T6W28 *chip);

/**
 * Write value to T6W28 chip port#0.
 * @param  value: value to write.
 * @param  *chip: The T6W28 chip.
 */
void t6W28W(u8 val, T6W28 *chip);

/**
 * Write value to T6W28 chip port#1.
 * @param  value: value to write.
 * @param  *chip: The T6W28 chip.
 */
void t6W28L_W(u8 val, T6W28 *chip);


#ifdef __cplusplus
}
#endif

#endif // T6W28_HEADER
