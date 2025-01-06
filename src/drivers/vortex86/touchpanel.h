#ifndef _TP_H_
#define _TP_H_

#ifdef __cplusplus
extern "C" {
#endif

void touchpanel_init(void);
bool get_touchXY(int* dataX, int* dataY);

#ifdef __cplusplus
}
#endif

#endif