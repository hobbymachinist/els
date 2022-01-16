#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_bore_dimension_setup(void);
void els_bore_dimension_start(void);
void els_bore_dimension_update(void);
void els_bore_dimension_stop(void);
bool els_bore_dimension_busy(void);

#ifdef __cplusplus
}
#endif

