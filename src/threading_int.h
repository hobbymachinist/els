#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_threading_int_setup(void);
void els_threading_int_start(void);
void els_threading_int_update(void);
void els_threading_int_stop(void);
bool els_threading_int_busy(void);

#ifdef __cplusplus
}
#endif

