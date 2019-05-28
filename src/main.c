// file auto generated

#include <stdlib.h>
#include "./lib/signalHandler/signalHandler.h"
#include "./lib/sharedMem/sharedMem.h"
#include "./lib/log/log.h"
#include "./lib/freeOnExit/freeOnExit.h"
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "./lib/config/config_arg.h"
#include "./lib/config/config_file.h"
#include "./lib/sharedMem/sharedMem.h"
#include "interface.h"


uint8_t stop = 0;

// INIT_FUNCTION
void functionExit ( void * arg )
{
	printf ( "%s\n", ( char * )arg );
	stop = 1;
}

int main ( int argc, char ** argv )
{
	detection_t * detection = NULL;
	uint32_t memKey = 665896;
	float offsetAngle = 0.0;
	int err = 0;

	char port[128] = { 0 };

	// INIT_VAR
	signalHandling signal =
	{
		.flag =
		{
			.Int = 1,
			.Quit = 1,
			.Term = 1
		},
		.Int =
		{
			.func = functionExit,
			.arg = "Int case\n"
		},
		.Quit =
		{
			.func = functionExit,
			.arg = "Quit case\n"
		},
		.Term =
		{
			.func = functionExit,
			.arg = "Term case\n"
		}
	};
 
	struct
	{
		#ifdef __LOG_H__
		uint8_t help:1,
			quiet:1,
			verbose:1,
			color:1,
			debug:1,
			term:1,
			file:1;
		#else
		uint8_t help:1,
			unused:6; // to have same allignement with and without debug/color/quiet flags
		#endif
	}
	flags = { 0 };
	char logFileName[ 512 ] = "";
 
	config_el config[] =
	{
		{ "ANGLE_OFFSET", cT ( float ), &offsetAngle, "angle offset from the 0" },
		{ "SHARED_MEME_KEY", cT ( uint32_t ), &memKey, "shared memory key" },
		{ "DEVICE", cT ( str ), port, "shared memory key" },
		{ NULL }
	};
 
	param_el param[] =
	{
		{ "--help", "-h", 0x01, cT ( bool ), &flags, "help" },
		#ifdef __LOG_H__
		{ "--quiet", "-q", 0x02, cT ( bool ), &flags, "quiet" },
		{ "--verbose", "-v", 0x04, cT ( bool ), &flags, "verbose" },
		{ "--color", "-c", 0x08, cT ( bool ), &flags, "color" },
		{ "--debug", "-d", 0x10, cT ( bool ), &flags, "debug" },
		{ "--term", "-lT", 0x20, cT ( bool ), &flags, "log on term" },
		{ "--file", "-lF", 0x40, cT ( bool ), &flags, "log in file" },
		{ "--logFileName", "-lFN", 1, cT ( str ), logFileName, "log file name" },
		#endif
		{ "--angleOffset", "-oA", 1, cT ( float ), &offsetAngle, "angle offset from the 0" },
		{ "--memKey", "-k", 1, cT ( uint32_t ), &memKey, "shared memory key" },
		{ "--port", "-p", 1, cT ( str ), port, "shared memory key" },
		{ NULL }
	};
 
	// INIT_CORE
	// INIT_SIGNALHANDLER
	signalHandlerInit ( &signal );
	// END_SIGNALHANDLER
	
	// INIT_FREEONEXIT
	if ( initFreeOnExit ( ) )
	{ // failure case
		logVerbose ( "freeOnExit error\n" );
		return ( __LINE__ );
	}
	// END_FREEONEXIT
	
	// INIT_CONFIG
	if ( readConfigFile ( "res/detection.config", config ) ||
		readConfigArgs ( argc, argv, config ) ||
		readParamArgs ( argc, argv, param ) )
	{
		printf ( "no config file\n" );
		return ( __LINE__ );
	}
	else if ( flags.help )
	{// configFile read successfully
		helpParamArgs ( param );
		helpConfigArgs ( config );
		return ( 0 );
	}
	// END_CONFIG
	// INIT_LOG
	#ifdef __CONFIG_DATA_H__
	logSetVerbose ( flags.verbose );
	logSetDebug ( flags.debug );
	logSetColor ( flags.color );
	logSetQuiet ( flags.quiet );
	logSetOutput ( flags.term, flags.file );
	logSetFileName ( logFileName );
	#else
	logSetVerbose ( 1 );
	logSetDebug ( 1 );
	logSetColor ( 1 );
	logSetQuiet ( 0 );
	logSetOutput ( 1, 1 );
	logSetFileName ( "log.txt" );
	#endif
	// END_LOG
	
	logVerbose ( "Init shared mem\n" );
	if ( err = getSharedMem ( ( void ** ) &detection, sizeof ( *detection ), memKey ), err )
	{
		logVerbose ( "sharedMem error %d\n", err );
		logVerbose ( " - error : %s\n", strerror ( errno ) );
		return ( __LINE__ );
	}
	
	logVerbose ( "Start lidar %s\n", port );
	if ( err = lidarUse ( port, 115200, offsetAngle, detection, &stop ), err )
	{
		logVerbose ( "lidar error %d\n", err );
		logVerbose ( " - error : %s\n", strerror ( errno ) );
		return ( __LINE__ );
	}

	// END_CORE
	logVerbose ( "code endded\n" );
	return ( 0 );
}

