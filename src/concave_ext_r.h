#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_concave_ext_r_setup(void);
void els_concave_ext_r_start(void);
void els_concave_ext_r_update(void);
void els_concave_ext_r_stop(void);
bool els_concave_ext_r_busy(void);

#ifdef __cplusplus
}
#endif

