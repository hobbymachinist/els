#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_bore_hole_setup(void);
void els_bore_hole_start(void);
void els_bore_hole_update(void);
void els_bore_hole_stop(void);
bool els_bore_hole_busy(void);

#ifdef __cplusplus
}
#endif

