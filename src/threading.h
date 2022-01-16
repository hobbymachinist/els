#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_threading_setup(void);
void els_threading_start(void);
void els_threading_update(void);
void els_threading_stop(void);
bool els_threading_busy(void);

#ifdef __cplusplus
}
#endif

