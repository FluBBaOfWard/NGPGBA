#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

void setupGUI(void);
void enterGUI(void);
void exitGUI(void);
void quickSelectGame(void);
void nullUINormal(int key);
void nullUIDebug(int key);
void resetConsole(void);

void uiNullNormal(void);
void uiAbout(void);
void uiLoadGame(void);

void debugIOUnimplR(u16 port);
void debugIOUnimplW(u8 val, u16 port);
void debugDivideError(void);
void debugUndefinedInstruction(void);
void debugCrashInstruction(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUI_HEADER
