#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "lib/rplidar/include/rplidar.h"
#include "interface.h"

#include "lib/freeOnExit/freeOnExit.h"

#define ANGLE(nodes,id) ( ( ( nodes[ id ].angle_q6_checkbit ) >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT ) / 64.0f )
#define DISTANCE(nodes,id) ( nodes[ id ].distance_q2 / 4.0f )

using namespace rp::standalone::rplidar;

static inline void evaluateDist ( const rplidar_response_measurement_node_t nodes[], const size_t id, float * const value )
{
	float d = DISTANCE ( nodes, id );

	if ( d < *value &&
		d > DISTANCE_MIN )
	{
		*value = d;
	}
}

int lidarUse ( const char * path, const uint32_t baudRate, const float offsetAngle, detection_t * detection, uint8_t *stop )
{
	u_result op_result;

	if ( !path ||
		!*path ||
		!detection ||
		!stop )
	{
		errno = EINVAL;
	}

	RPlidarDriver * drv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
	if ( !drv )
	{
		return ( __LINE__ );
	}

	// make connection...
	if ( IS_FAIL ( drv->connect ( path, baudRate ) ) )
	{
		RPlidarDriver::DisposeDriver ( drv );
		return ( __LINE__ );
	}

	rplidar_response_device_info_t devinfo;

	// retrieving the device info
	op_result = drv->getDeviceInfo(devinfo);

	if ( IS_FAIL ( op_result ) )
	{
		RPlidarDriver::DisposeDriver ( drv );
		return ( __LINE__ );
	}

	drv->startMotor ( );
	// start scan...
	drv->startScan ( );

	while ( !*stop )
	{
		float distance = 1000000.0;

		size_t nbEls = 360;
		rplidar_response_measurement_node_t nodes[ nbEls ];
		op_result = drv->grabScanData ( nodes, nbEls );

		if ( IS_OK ( op_result ) )
		{
			drv->ascendScanData ( nodes, nbEls );
			for ( size_t id = 0; id < nbEls ; ++id )
			{
				float angle = ANGLE ( nodes, id ) + offsetAngle;

				if ( angle > 360 )
				{
					angle -= 360;
				}
				else if ( angle < 0 )
				{
					angle += 360;
				}

				if ( ( detection->dir & DIR_FORWARD ) == DIR_FORWARD &&
					( ( angle > 315
					&&  angle <= 360 )
					|| angle >= 0
					&& angle < 45 ) )
				{
					evaluateDist ( nodes, id, &distance );
				}

				if ( ( detection->dir & DIR_RIGHT ) == DIR_RIGHT &&
					angle >=45 &&
					angle <= 135 )
				{
					evaluateDist ( nodes, id, &distance );
				}

				if ( ( detection->dir & DIR_LEFT ) == DIR_LEFT &&
					angle >= 225 &&
					angle <= 315 )
				{
					evaluateDist ( nodes, id, &distance );
				}

				if ( ( detection->dir & DIR_BACKWARD ) == DIR_BACKWARD &&
					angle > 135 &&
					angle < 225 )
				{
					evaluateDist ( nodes, id, &distance );
				}
			}

			printf ( "%f\r", distance );

			if ( detection->dir != DIR_NONE )
			{
				detection->distance = distance;
			}
		}
	}

	drv->stop ( );
	drv->stopMotor ( );

	RPlidarDriver::DisposeDriver ( drv );

	return ( 0 );
}
