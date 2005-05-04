/*
*	serial I/O, driver
*	Nicholas Christopoulos
*/

#include "sys.h"
#include "device.h"
#include "pproc.h"
#include "match.h"
#if defined(_PalmOS)
	#include <FileStream.h>
	#include <SerialMgrOld.h>
#elif defined(_VTOS)
	typedef FILE *	FileHand;
#else
	#include <errno.h>

	#if defined(_UnixOS) && !defined(__MINGW32__)
	#include <sys/time.h>
	#include <termios.h>
	#include <unistd.h>
	#elif defined(_DOS)
	#include "dos-djgpp/svasync/svasync.h"
	#endif

	#include <dirent.h>		// POSIX standard (Note: Borland C++ compiler supports it; VC no)

	typedef int FileHand;

	#include "fs_stream.h"
#endif
#include "fs_serial.h"

#if defined(__CYGWIN__) || defined(__MINGW32__)
#include <windows.h>
#endif

/*
*/
int		serial_open(dev_file_t *f)
{
	#if defined(_PalmOS)
	///////////////////////////////////////////////////////////////////////////////////////////
	//	PalmOS
	SysLibFind("Serial Library", &f->libHandle);
	f->last_error = SerOpen(f->libHandle, f->port, f->devspeed);
	f->handle = 1;
	if	( f->last_error )
		rt_raise("SEROPEN() ERROR %d", f->last_error);
	return (f->last_error == 0);

    #elif defined(_UnixOS) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	///////////////////////////////////////////////////////////////////////////////////////////
	//	Unix
	sprintf(f->name, "/dev/ttyS%d", f->port);

	f->handle = open(f->name, O_RDWR | O_NOCTTY);
	if	( f->handle < 0 )
		err_file((f->last_error = errno));

	tcgetattr(f->handle, &f->oldtio); /* save current port settings */
	bzero(&f->newtio, sizeof(f->newtio));
	f->newtio.c_cflag = f->devspeed | CRTSCTS | CS8 | CLOCAL | CREAD;
	f->newtio.c_iflag = IGNPAR;
	f->newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	f->newtio.c_lflag = 0;
	f->newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	f->newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */
	tcflush(f->handle, TCIFLUSH);
	tcsetattr(f->handle, TCSANOW, &f->newtio);
	return (f->handle >= 0);

	#elif defined(_DOS)
	///////////////////////////////////////////////////////////////////////////////////////////
	//	DOS
	//	(1-comport per time)
	sprintf(f->name, "COM%d", f->port-1);

	f->handle = f->port-1;
	if	( SVAsyncInit(f->handle) )	
		f->handle = -1;
	else	{
		SVAsyncFifoInit();
		SVAsyncSet(f->devspeed, BITS_8 | STOP_1 | NO_PARITY);
		SVAsyncHand(DTR | RTS);
		}

	if	( f->handle < 0 )
		err_file((f->last_error = errno));
	return (f->handle >= 0);
    #elif defined(_Win32) || defined(__CYGWIN__)
	///////////////////////////////////////////////////////////////////////////////////////////
	//	Win32
	DCB		dcb;
	HANDLE	hCom;
	DWORD	dwer;

	sprintf(f->name, "COM%d", f->port);

	hCom = CreateFile(f->name, GENERIC_READ | GENERIC_WRITE,
		    0, NULL, OPEN_EXISTING, 0, NULL);

	if ( hCom == INVALID_HANDLE_VALUE )	{
    	dwer = GetLastError();
        if	( dwer != 5 )
	        rt_raise("SERIALFS: CreateFile() failed (%d)", dwer);
        else
	        rt_raise("SERIALFS: ACCESS DENIED");
        return 0;
        }

	if ( !GetCommState(hCom, &dcb) ) {
        rt_raise("SERIALFS: GetCommState() failed (%d)", GetLastError());
        return 0;
        }

	dcb.BaudRate = f->devspeed;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if ( !SetCommState(hCom, &dcb) ) {
        rt_raise("SERIALFS: SetCommState() failed (%d)", GetLastError());
        return 0;
    	}

    f->handle = (FileHand) hCom;
	return 1;
	#else
	///////////////////////////////////////////////////////////////////////////////////////////
	//	ERROR
	err_unsup();
	return 0;	// failed
	#endif
}

/*
*/
int		serial_close(dev_file_t *f)
{
	#if defined(_PalmOS)
	f->last_error = SerClose(f->libHandle);
	f->handle = -1;
	if	( f->last_error )
		rt_raise("SERCLOSE() ERROR %d", f->last_error);
	return (f->last_error == 0);

	#elif defined(_UnixOS) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	tcsetattr(f->handle, TCSANOW, &f->oldtio);
	close(f->handle);
	f->handle = -1;
	return 1;

	#elif defined(_DOS)
	SVAsyncStop();
	f->handle = (FileHand) -1;
	return 1;

    #elif defined(_Win32) || defined(__CYGWIN__) || defined(__MINGW32__)
    CloseHandle((HANDLE) f->handle);
	f->handle = -1;
	return 1;

	#else
	return 0;
	#endif
}

/*
*/
int		serial_write(dev_file_t *f, byte *data, dword size)
{
	#if defined(_PalmOS)
	SerSend(f->libHandle, data, size, &f->last_error);
	if	( f->last_error )
		rt_raise("SERSEND() ERROR %d", f->last_error);
	return (f->last_error == 0);

	#elif defined(_UnixOS)
	return stream_write(f, data, size);
	#elif defined(_DOS)
	int		i;

	for ( i = 0; i < size; i ++ )	
		SVAsyncOut(data[i]);
	return size;
    #elif defined(_Win32)
    DWORD	bytes;

	f->last_error = !WriteFile((HANDLE) f->handle, data, size, &bytes, NULL);
    return bytes;
	#else
	return 0;
	#endif
}

/*
*/
int		serial_read(dev_file_t *f, byte *data, dword size)
{
	#if defined(_PalmOS)
	dword	num;
	int		ev;

	do	{
		f->last_error = SerReceiveCheck(f->libHandle, &num);
		ev = dev_events(0);

		if	( f->last_error || prog_error || ev < 0 )	{
			SerClose(f->libHandle);
			f->handle = -1;
			break;
			}
		if	( num >= size )
			break;
		} while ( 1 );

	if	( num >= size )
		f->last_error = SerReceive10(f->libHandle, data, size, -1);

	if	( f->last_error )
		rt_raise("SERRECEIVE10() ERROR %d", f->last_error);

	return (f->last_error == 0);

	#elif defined(_UnixOS)
	return stream_read(f, data, size);
	#elif defined(_DOS)
	int		i;

	for ( i = 0; i < size; i ++ )	{
		while ( SVAsyncInStat() == 0 )	usleep(50000);
		data[i] = SVAsyncIn();
		}
	return size;
    #elif defined(_Win32)
    DWORD	bytes;

    f->last_error = !ReadFile((HANDLE) f->handle, data, size, &bytes, NULL);
    return bytes;
	#else
	return 0;
	#endif
}

/*
*	Returns the number of the available data on serial port
*/
dword	serial_length(dev_file_t *f)
{
	#if defined(_PalmOS)
	dword	num;

	f->last_error = SerReceiveCheck(f->libHandle, &num);
	if	( f->last_error )	{
		SerClose(f->libHandle);
		f->handle = -1;
		num = 0;
		}
	return num;

	///////////////////////////////////////////////////////////////////////////////////////////
	#elif defined(_UnixOS)
	fd_set	readfs;
	struct timeval tv;
	int		res;

	FD_ZERO(&readfs);
	FD_SET(f->handle, &readfs);

	tv.tv_usec = 250;  /* milliseconds */
    tv.tv_sec  = 0;    /* seconds */

	res = select(f->handle+1, &readfs, NULL, NULL, &tv);
	if ( FD_ISSET(f->handle, &readfs) )
		return 1;

	return 0;
	#elif defined(_DOS)
	return SVAsyncInStat();
	#elif defined(_Win32)
    COMSTAT	cs;
    DWORD	de = CE_BREAK;

    ClearCommError((HANDLE) f->handle, &de, &cs);
    return cs.cbInQue;
    #else
	return 0;
	#endif
}

/*
*	Returns true (EOF) if the connection is broken
*/
dword	serial_eof(dev_file_t *f)
{
	#if defined(_PalmOS)
	Boolean	a, b;
	return (SerGetStatus(f->libHandle, &a, &b) != 0);
	#elif defined(_DOS)
	return (SVAsyncInStat() == 0);
	#else
    return f->last_error;
	#endif
}

