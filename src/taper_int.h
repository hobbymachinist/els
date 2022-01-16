#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_taper_int_setup(void);
void els_taper_int_start(void);
void els_taper_int_update(void);
void els_taper_int_stop(void);
bool els_taper_int_busy(void);

#ifdef __cplusplus
}
#endif

