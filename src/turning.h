#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_turning_setup(void);
void els_turning_start(void);
void els_turning_update(void);
void els_turning_stop(void);
bool els_turning_busy(void);

#ifdef __cplusplus
}
#endif

