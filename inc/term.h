#ifndef TERM_H
#define TERM_H

int TERM_enableRawMode(void);
int TERM_disableRawMode(void);

int TERM_getch(void);
int TERM_getchNB(char* key);

#endif