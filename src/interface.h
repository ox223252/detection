#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <stdint.h>
#include "structDetection.h"

#ifdef __cplusplus
extern "C" {
#endif

int lidarUse ( const char * path, const uint32_t baudRate, const float offsetAngle, detection_t * detection, uint8_t *stop );

#ifdef __cplusplus
}
#endif

#endif