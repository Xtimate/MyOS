#ifndef USERMODE_H
#define USERMODE_H

void jump_usermode(void (*func)(), unsigned int user_stack);

#endif
