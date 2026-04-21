#ifndef WINGXA_H
#define WINGXA_H

void gwinsize(int width, int height);
void ginit(int mode);
void gsetorg(int x, int y);
int keypress(void);
void gcolor(int r, int g, int b);
void grect(int x1, int y1, int x2, int y2);
void swapbuffers(void);
int save_screen(const char *filename);
char *itoa(int value, char *buffer, int base);

#endif
