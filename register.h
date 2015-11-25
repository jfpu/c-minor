#ifndef REGISTER_H
#define REGISTER_H

const char *register_name(int r);
const char *param_register_name(int i);
int register_alloc();
void register_free(int r);

#endif
