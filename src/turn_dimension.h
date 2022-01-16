#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void els_turn_dimension_setup(void);
void els_turn_dimension_start(void);
void els_turn_dimension_update(void);
void els_turn_dimension_stop(void);
bool els_turn_dimension_busy(void);

#ifdef __cplusplus
}
#endif

