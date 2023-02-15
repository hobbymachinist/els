#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_parting_setup(void);
void els_parting_start(void);
void els_parting_update(void);
void els_parting_stop(void);
bool els_parting_busy(void);

#ifdef __cplusplus
}
#endif

