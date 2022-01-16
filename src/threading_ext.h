#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_threading_ext_setup(void);
void els_threading_ext_start(void);
void els_threading_ext_update(void);
void els_threading_ext_stop(void);
bool els_threading_ext_busy(void);

#ifdef __cplusplus
}
#endif

