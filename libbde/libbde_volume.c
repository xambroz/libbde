/*
 * Volume functions
 *
 * Copyright (C) 2011-2012, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libbde_debug.h"
#include "libbde_definitions.h"
#include "libbde_io_handle.h"
#include "libbde_libbfio.h"
#include "libbde_libcerror.h"
#include "libbde_libcnotify.h"
#include "libbde_libcstring.h"
#include "libbde_libfcache.h"
#include "libbde_libfdata.h"
#include "libbde_metadata.h"
#include "libbde_password.h"
#include "libbde_recovery.h"
#include "libbde_sector_data.h"
#include "libbde_volume.h"

#include "bde_metadata.h"

/* Initialize a volume
 * Make sure the value volume is pointing to is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_initialize(
     libbde_volume_t **volume,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_initialize";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( *volume != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume value already set.",
		 function );

		return( -1 );
	}
	internal_volume = memory_allocate_structure(
	                   libbde_internal_volume_t );

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create volume.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_volume,
	     0,
	     sizeof( libbde_internal_volume_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear volume.",
		 function );

		memory_free(
		 internal_volume );

		return( -1 );
	}
	if( libbde_metadata_initialize(
	     &( internal_volume->primary_metadata ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create primary metadata.",
		 function );

		goto on_error;
	}
	if( libbde_metadata_initialize(
	     &( internal_volume->secondary_metadata ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create secondary metadata.",
		 function );

		goto on_error;
	}
	if( libbde_metadata_initialize(
	     &( internal_volume->tertiary_metadata ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create tertiary metadata.",
		 function );

		goto on_error;
	}
	if( libbde_io_handle_initialize(
	     &( internal_volume->io_handle ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create IO handle.",
		 function );

		goto on_error;
	}
	*volume = (libbde_volume_t *) internal_volume;

	return( 1 );

on_error:
	if( internal_volume != NULL )
	{
		if( internal_volume->tertiary_metadata != NULL )
		{
			libbde_metadata_free(
			 &( internal_volume->tertiary_metadata ),
			 NULL );
		}
		if( internal_volume->secondary_metadata != NULL )
		{
			libbde_metadata_free(
			 &( internal_volume->secondary_metadata ),
			 NULL );
		}
		if( internal_volume->primary_metadata != NULL )
		{
			libbde_metadata_free(
			 &( internal_volume->primary_metadata ),
			 NULL );
		}
		memory_free(
		 internal_volume );
	}
	return( -1 );
}

/* Frees a volume
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_free(
     libbde_volume_t **volume,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_free";
	int result                                = 1;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	if( *volume != NULL )
	{
		internal_volume = (libbde_internal_volume_t *) *volume;

		if( internal_volume->file_io_handle != NULL )
		{
			if( libbde_volume_close(
			     *volume,
			     error ) != 0 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_CLOSE_FAILED,
				 "%s: unable to close volume.",
				 function );

				result = -1;
			}
		}
		*volume = NULL;

		if( libbde_metadata_free(
		     &( internal_volume->primary_metadata ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free primary metadata.",
			 function );

			result = -1;
		}
		if( libbde_metadata_free(
		     &( internal_volume->secondary_metadata ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free secondary metadata.",
			 function );

			result = -1;
		}
		if( libbde_metadata_free(
		     &( internal_volume->tertiary_metadata ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free tertiary metadata.",
			 function );

			result = -1;
		}
		if( libbde_io_handle_free(
		     &( internal_volume->io_handle ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free IO handle.",
			 function );

			result = -1;
		}
		memory_free(
		 internal_volume );
	}
	return( result );
}

/* Signals the volume to abort its current activity
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_signal_abort(
     libbde_volume_t *volume,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_signal_abort";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->abort = 1;

	return( 1 );
}

/* Opens a volume
 * Returns 1 if successful, 0 if the keys could not be read or -1 on error
 */
int libbde_volume_open(
     libbde_volume_t *volume,
     const char *filename,
     int access_flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle          = NULL;
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_open";
	size_t filename_length                    = 0;
	int result                                = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBBDE_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	filename_length = libcstring_narrow_string_length(
	                   filename );

	if( filename_length == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		goto on_error;
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set track offsets read in file IO handle.",
                 function );

		goto on_error;
	}
#endif
	if( libbfio_file_set_name(
	     file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		goto on_error;
	}
	result = libbde_volume_open_file_io_handle(
	          volume,
	          file_io_handle,
	          access_flags,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open volume: %s.",
		 function,
		 filename );

		goto on_error;
	}
	internal_volume->file_io_handle_created_in_library = 1;

	return( result );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
        return( -1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Opens a volume
 * Returns 1 if successful, 0 if the keys could not be read or -1 on error
 */
int libbde_volume_open_wide(
     libbde_volume_t *volume,
     const wchar_t *filename,
     int access_flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle          = NULL;
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_open_wide";
	size_t filename_length                    = 0;
	int result                                = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBBDE_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	filename_length = libcstring_wide_string_length(
	                   filename );

	if( filename_length == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		goto on_error;
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libbfio_handle_set_track_offsets_read(
	     file_io_handle,
	     1,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set track offsets read in file IO handle.",
                 function );

		goto on_error;
	}
#endif
	if( libbfio_file_set_name_wide(
	     file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		goto on_error;
	}
	result = libbde_volume_open_file_io_handle(
	          volume,
	          file_io_handle,
	          access_flags,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open volume: %ls.",
		 function,
		 filename );

		goto on_error;
	}
	internal_volume->file_io_handle_created_in_library = 1;

	return( result );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
        return( -1 );
}

#endif

/* Opens a volume using a Basic File IO (bfio) handle
 * Returns 1 if successful, 0 if the keys could not be read or -1 on error
 */
int libbde_volume_open_file_io_handle(
     libbde_volume_t *volume,
     libbfio_handle_t *file_io_handle,
     int access_flags,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_open_file_io_handle";
	int bfio_access_flags                     = 0;
	int file_io_handle_is_open                = 0;
	int result                                = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( file_io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file IO handle.",
		 function );

		return( -1 );
	}
	if( ( ( access_flags & LIBBDE_ACCESS_FLAG_READ ) == 0 )
	 && ( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported access flags.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBBDE_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( ( access_flags & LIBBDE_ACCESS_FLAG_READ ) != 0 )
	{
		bfio_access_flags = LIBBFIO_ACCESS_FLAG_READ;
	}
	file_io_handle_is_open = libbfio_handle_is_open(
	                          file_io_handle,
	                          error );

	if( file_io_handle_is_open == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file.",
		 function );

		return( -1 );
	}
	else if( file_io_handle_is_open == 0 )
	{
		if( libbfio_handle_open(
		     file_io_handle,
		     bfio_access_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_OPEN_FAILED,
			 "%s: unable to open file IO handle.",
			 function );

			return( -1 );
		}
	}
	internal_volume->file_io_handle = file_io_handle;

	result = libbde_volume_open_read(
	          internal_volume,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read from volume handle.",
		 function );

		if( file_io_handle_is_open == 0 )
		{
			libbfio_handle_close(
			 file_io_handle,
			 error );
		}
		return( -1 );
	}
	return( result );
}

/* Closes a volume
 * Returns 0 if successful or -1 on error
 */
int libbde_volume_close(
     libbde_volume_t *volume,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_close";
	int result                                = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle_created_in_library != 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			if( libbde_debug_print_read_offsets(
			     internal_volume->file_io_handle,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				 "%s: unable to print the read offsets.",
				 function );
			}
		}
#endif
		if( libbfio_handle_close(
		     internal_volume->file_io_handle,
		     error ) != 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_CLOSE_FAILED,
			 "%s: unable to close file IO handle.",
			 function );

			result = -1;
		}
		if( libbfio_handle_free(
		     &( internal_volume->file_io_handle ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file IO handle.",
			 function );

			result = -1;
		}
	}
	internal_volume->file_io_handle                    = NULL;
	internal_volume->file_io_handle_created_in_library = 0;

	if( libfdata_vector_free(
	     &( internal_volume->sectors_vector ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free sectors vector.",
		 function );

		result = -1;
	}
	if( libfcache_cache_free(
	     &( internal_volume->sectors_cache ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free sectors cache.",
		 function );

		result = -1;
	}
	return( result );
}

/* Opens a volume for reading
 * Returns 1 if successful, 0 if the keys could not be read or -1 on error
 */
int libbde_volume_open_read(
     libbde_internal_volume_t *internal_volume,
     libcerror_error_t **error )
{
	uint8_t *startup_key_identifier    = NULL;
	static char *function              = "libbde_volume_open_read";
	size_t startup_key_identifier_size = 0;
	int result                         = 0;

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal volume.",
		 function );

		return( -1 );
	}
	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->sectors_vector != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid internal file - sectors vector already set.",
		 function );

		return( -1 );
	}
	if( internal_volume->sectors_cache != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid internal file - sectors cache already set.",
		 function );

		return( -1 );
	}
	if( ( internal_volume->external_key_metadata != NULL )
	 && ( internal_volume->external_key_metadata->startup_key_external_key != NULL ) )
	{
		startup_key_identifier      = internal_volume->external_key_metadata->startup_key_external_key->identifier;
		startup_key_identifier_size = 16;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading BitLocker volume header:\n" );
	}
#endif
	if( libbde_io_handle_read_volume_header(
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     0,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read volume header.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading BitLocker primary metadata block:\n" );
	}
#endif
	if( libbde_metadata_read_block(
	     internal_volume->primary_metadata,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     internal_volume->io_handle->first_metadata_offset,
	     startup_key_identifier,
	     startup_key_identifier_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read primary metadata block.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading BitLocker secondary metadata block:\n" );
	}
#endif
	if( libbde_metadata_read_block(
	     internal_volume->secondary_metadata,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     internal_volume->io_handle->second_metadata_offset,
	     startup_key_identifier,
	     startup_key_identifier_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read secondary metadata block.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading BitLocker tertiary metadata block:\n" );
	}
#endif
	if( libbde_metadata_read_block(
	     internal_volume->tertiary_metadata,
	     internal_volume->io_handle,
	     internal_volume->file_io_handle,
	     internal_volume->io_handle->third_metadata_offset,
	     startup_key_identifier,
	     startup_key_identifier_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read tertiary metadata block.",
		 function );

		goto on_error;
	}
	result = libbde_volume_open_read_keys_from_metadata(
	          internal_volume,
	          internal_volume->primary_metadata,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read keys from primary metadata.",
		 function );

		goto on_error;
	}
	if( result == 0 )
	{
		result = libbde_volume_open_read_keys_from_metadata(
			  internal_volume,
			  internal_volume->secondary_metadata,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read keys from secondary metadata.",
			 function );

			goto on_error;
		}
	}
	if( result == 0 )
	{
		result = libbde_volume_open_read_keys_from_metadata(
			  internal_volume,
			  internal_volume->tertiary_metadata,
			  error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read keys from tertiary metadata.",
			 function );

			goto on_error;
		}
	}
	if( result != 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading unencrypted volume header:\n" );
		}
#endif
		if( libbde_io_handle_read_unencrypted_volume_header(
		     internal_volume->io_handle,
		     internal_volume->file_io_handle,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read unencrypted volume header.",
			 function );

			goto on_error;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: volume size\t\t\t\t\t: %" PRIu64 "\n",
			 function,
			 internal_volume->io_handle->volume_size );

			libcnotify_printf(
			 "\n" );
		}
#endif
		/* TODO clone function ? */
		if( libfdata_vector_initialize(
		     &( internal_volume->sectors_vector ),
		     (size64_t) internal_volume->io_handle->bytes_per_sector,
		     (intptr_t *) internal_volume->io_handle,
		     NULL,
		     NULL,
		     &libbde_io_handle_read_sector,
		     LIBFDATA_FLAG_IO_HANDLE_NON_MANAGED,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create sectors vector.",
			 function );

			goto on_error;
		}
		if( libfdata_vector_append_segment(
		     internal_volume->sectors_vector,
		     0,
		     internal_volume->io_handle->volume_size,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
			 "%s: unable to append segment to sectors vector.",
			 function );

			goto on_error;
		}
		if( libfcache_cache_initialize(
		     &( internal_volume->sectors_cache ),
		     LIBBDE_MAXIMUM_CACHE_ENTRIES_SECTORS,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create sectors cache.",
			 function );

			goto on_error;
		}
	}
	return( result );

on_error:
	if( internal_volume->sectors_cache != NULL )
	{
		libfcache_cache_free(
		 &( internal_volume->sectors_cache ),
		 NULL );
	}
	if( internal_volume->sectors_vector != NULL )
	{
		libfdata_vector_free(
		 &( internal_volume->sectors_vector ),
		 NULL );
	}
	return( -1 );
}

/* Reads the keys from the metadata when opening the volume for reading
 * Returns 1 if successful, 0 if not or -1 on error
 */
int libbde_volume_open_read_keys_from_metadata(
     libbde_internal_volume_t *internal_volume,
     libbde_metadata_t *metadata,
     libcerror_error_t **error )
{
	uint8_t full_volume_encryption_key[ 32 ];
	uint8_t tweak_key[ 32 ];
	uint8_t volume_master_key[ 32 ];

	uint8_t *external_key          = NULL;
	static char *function          = "libbde_volume_open_read_keys_from_metadata";
	off64_t volume_header_offset   = 0;
	size64_t volume_header_size    = 0;
	size64_t encrypted_volume_size = 0;
	size_t external_key_size       = 0;
	uint32_t encryption_method     = 0;
	int result                     = 0;

	if( internal_volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal volume.",
		 function );

		return( -1 );
	}
	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( metadata == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid metadata.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     full_volume_encryption_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear full volume encryption key.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     tweak_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear tweak key.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     volume_master_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to volume master key.",
		 function );

		return( -1 );
	}
	if( ( internal_volume->external_key_metadata != NULL )
	 && ( internal_volume->external_key_metadata->startup_key_external_key != NULL )
	 && ( internal_volume->external_key_metadata->startup_key_external_key->key != NULL ) )
	{
		external_key      = internal_volume->external_key_metadata->startup_key_external_key->key->data;
		external_key_size = internal_volume->external_key_metadata->startup_key_external_key->key->data_size;
	}
	encrypted_volume_size = metadata->encrypted_volume_size;
	volume_header_offset  = metadata->volume_header_offset;
	volume_header_size    = metadata->volume_header_size;
	encryption_method     = metadata->encryption_method;

	result = libbde_metadata_get_volume_master_key(
	          metadata,
	          internal_volume->io_handle,
	          external_key,
	          external_key_size,
	          volume_master_key,
	          32,
	          error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve volume master key from metadata.",
		 function );

		goto on_error;
	}
	else if( result != 0 )
	{
		result = libbde_metadata_get_full_volume_encryption_key(
		          metadata,
		          volume_master_key,
		          32,
		          full_volume_encryption_key,
		          32,
		          tweak_key,
		          32,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve full volume encryption key from metadata.",
			 function );

			goto on_error;
		}
	}
	if( result != 0 )
	{
		internal_volume->io_handle->encrypted_volume_size = encrypted_volume_size;
		internal_volume->io_handle->volume_header_offset  = volume_header_offset;
		internal_volume->io_handle->volume_header_size    = volume_header_size;

		if( libbde_encryption_initialize(
		     &( internal_volume->io_handle->encryption_context ),
		     encryption_method,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create encryption context.",
			 function );

			goto on_error;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: volume master key:\n",
			 function );
			libcnotify_print_data(
			 volume_master_key,
			 32,
			 0 );

			libcnotify_printf(
			 "%s: full volume encryption key:\n",
			 function );
			libcnotify_print_data(
			 full_volume_encryption_key,
			 32,
			 0 );

			libcnotify_printf(
			 "%s: tweak key:\n",
			 function );
			libcnotify_print_data(
			 tweak_key,
			 32,
			 0 );
		}
#endif
		if( libbde_encryption_set_keys(
		     internal_volume->io_handle->encryption_context,
		     full_volume_encryption_key,
		     tweak_key,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set keys in encryption context.",
			 function );

			goto on_error;
		}
	}
	if( memory_set(
	     full_volume_encryption_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear full volume encryption key.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     tweak_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear tweak key.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     volume_master_key,
	     0,
	     32 ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to volume master key.",
		 function );

		goto on_error;
	}
	internal_volume->encryption_method = encryption_method;

	return( result );

on_error:
	memory_set(
	 full_volume_encryption_key,
	 0,
	 32 );

	memory_set(
	 tweak_key,
	 0,
	 32 );

	memory_set(
	 volume_master_key,
	 0,
	 32 );

	return( -1 );
}

/* Reads data at the current offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libbde_volume_read_buffer(
         libbde_volume_t *volume,
         void *buffer,
         size_t buffer_size,
         libcerror_error_t **error )
{
	libbde_sector_data_t *sector_data         = NULL;
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_read_buffer";
	size_t buffer_offset                      = 0;
	size_t read_size                          = 0;
	size_t sector_data_offset                 = 0;
	ssize_t total_read_count                  = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->sectors_vector == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing sectors vector.",
		 function );

		return( -1 );
	}
	if( internal_volume->sectors_cache == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing sectors cache.",
		 function );

		return( -1 );
	}
	if( internal_volume->io_handle->current_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid internal volume - invalid IO handle - current offset value out of bounds.",
		 function );

		return( -1 );
	}
	if( buffer == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid buffer.",
		 function );

		return( -1 );
	}
	if( buffer_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid buffer size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( (size64_t) internal_volume->io_handle->current_offset >= internal_volume->io_handle->volume_size )
	{
		return( 0 );
	}
	if( (size64_t) ( internal_volume->io_handle->current_offset + buffer_size ) >= internal_volume->io_handle->volume_size )
	{
		buffer_size = (size_t) ( internal_volume->io_handle->volume_size - internal_volume->io_handle->current_offset );
	}
	sector_data_offset = (size_t) ( internal_volume->io_handle->current_offset % internal_volume->io_handle->bytes_per_sector );

	while( buffer_size > 0 )
	{
		if( libfdata_vector_get_element_value_at_offset(
		     internal_volume->sectors_vector,
		     internal_volume->file_io_handle,
		     internal_volume->sectors_cache,
		     internal_volume->io_handle->current_offset,
		     (intptr_t **) &sector_data,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve sector data at offset: %" PRIi64 ".",
			 function,
			 internal_volume->io_handle->current_offset );

			return( -1 );
		}
		if( sector_data == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			 "%s: missing sector data at offset: %" PRIi64 ".",
			 function,
			 internal_volume->io_handle->current_offset );

			return( -1 );
		}
		read_size = sector_data->data_size - sector_data_offset;

		if( read_size > buffer_size )
		{
			read_size = buffer_size;
		}
		if( read_size == 0 )
		{
			break;
		}
		if( memory_copy(
		     &( ( (uint8_t *) buffer )[ buffer_offset ] ),
		     &( ( sector_data->data )[ sector_data_offset ] ),
		     read_size ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to copy sector data to buffer.",
			 function );

			return( -1 );
		}
		buffer_offset     += read_size;
		buffer_size       -= read_size;
		total_read_count  += (ssize_t) read_size;
		sector_data_offset = 0;

		internal_volume->io_handle->current_offset += (off64_t) read_size;

		if( (size64_t) internal_volume->io_handle->current_offset >= internal_volume->io_handle->volume_size )
		{
			break;
		}
		if( internal_volume->io_handle->abort != 0 )
		{
			break;
		}
	}
	return( total_read_count );
}

/* Reads (media) data at a specific offset
 * Returns the number of bytes read or -1 on error
 */
ssize_t libbde_volume_read_random(
         libbde_volume_t *volume,
         void *buffer,
         size_t buffer_size,
         off64_t offset,
         libcerror_error_t **error )
{
	static char *function = "libbde_volume_read_random";
	ssize_t read_count    = 0;

	if( libbde_volume_seek_offset(
	     volume,
	     offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		return( -1 );
	}
	read_count = libbde_volume_read_buffer(
	              volume,
	              buffer,
	              buffer_size,
	              error );

	if( read_count < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read buffer.",
		 function );

		return( -1 );
	}
	return( read_count );
}

#ifdef TODO

/* Writes (media) data at the current offset
 * Returns the number of input bytes written, 0 when no longer bytes can be written or -1 on error
 */
ssize_t libbde_volume_write_buffer(
         libbde_volume_t *volume,
         void *buffer,
         size_t buffer_size,
         libcerror_error_t **error )
{
	return( -1 );
}

/* Writes (media) data at a specific offset,
 * Returns the number of input bytes written, 0 when no longer bytes can be written or -1 on error
 */
ssize_t libbde_volume_write_random(
         libbde_volume_t *volume,
         const void *buffer,
         size_t buffer_size,
         off64_t offset,
         libcerror_error_t **error )
{
	static char *function = "libbde_volume_write_random";
	ssize_t write_count   = 0;

	if( libbde_volume_seek_offset(
	     volume,
	     offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		return( -1 );
	}
	write_count = libbde_volume_write_buffer(
	               volume,
	               buffer,
	               buffer_size,
	               error );

	if( write_count < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write buffer.",
		 function );

		return( -1 );
	}
	return( write_count );
}

#endif

/* Seeks a certain offset of the data
 * Returns the offset if seek is successful or -1 on error
 */
off64_t libbde_volume_seek_offset(
         libbde_volume_t *volume,
         off64_t offset,
         int whence,
         libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_seek_offset";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( ( whence != SEEK_CUR )
	 && ( whence != SEEK_END )
	 && ( whence != SEEK_SET ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported whence.",
		 function );

		return( -1 );
	}
	if( whence == SEEK_CUR )
	{	
		offset += internal_volume->io_handle->current_offset;
	}
	else if( whence == SEEK_END )
	{	
		offset += (off64_t) internal_volume->io_handle->volume_size;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: seeking media data offset: %" PRIi64 ".\n",
		 function,
		 offset );
	}
#endif
	if( offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid offset value out of bounds.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->current_offset = offset;

	return( offset );
}

/* Retrieves the size
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_get_size(
     libbde_volume_t *volume,
     size64_t *size,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_get_size";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid size.",
		 function );

		return( -1 );
	}
	*size = internal_volume->io_handle->volume_size;

	return( 1 );
}

/* Retrieves the encryption method
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_get_encryption_method(
     libbde_volume_t *volume,
     uint32_t *encryption_method,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_get_encryption_method";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( encryption_method == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid encryption method.",
		 function );

		return( -1 );
	}
	*encryption_method = internal_volume->encryption_method;

	return( 1 );
}

/* Sets an UTF-8 formatted password
 * This function needs to be used before one of the open functions
 * Returns 1 if successful, 0 if password is invalid or -1 on error
 */
int libbde_volume_set_utf8_password(
     libbde_volume_t *volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_set_utf8_password";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( libbde_utf8_password_calculate_hash(
	     utf8_string,
	     utf8_string_length,
	     internal_volume->io_handle->password_hash,
	     32,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set password hash.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->password_is_set = 1;

	return( 1 );
}

/* Sets an UTF-16 formatted password
 * This function needs to be used before one of the open functions
 * Returns 1 if successful, 0 if password is invalid or -1 on error
 */
int libbde_volume_set_utf16_password(
     libbde_volume_t *volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_set_utf16_password";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( libbde_utf16_password_calculate_hash(
	     utf16_string,
	     utf16_string_length,
	     internal_volume->io_handle->password_hash,
	     32,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set password hash.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->password_is_set = 1;

	return( 1 );
}

/* Sets an UTF-8 formatted recovery password
 * This function needs to be used before one of the open functions
 * Returns 1 if successful, 0 if recovery password is invalid or -1 on error
 */
int libbde_volume_set_utf8_recovery_password(
     libbde_volume_t *volume,
     const uint8_t *utf8_string,
     size_t utf8_string_length,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_set_utf8_recovery_password";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( libbde_utf8_recovery_password_calculate_hash(
	     utf8_string,
	     utf8_string_length,
	     internal_volume->io_handle->recovery_password_hash,
	     32,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set recovery password hash.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->recovery_password_is_set = 1;

	return( 1 );
}

/* Sets an UTF-16 formatted recovery password
 * This function needs to be used before one of the open functions
 * Returns 1 if successful, 0 if recovery password is invalid or -1 on error
 */
int libbde_volume_set_utf16_recovery_password(
     libbde_volume_t *volume,
     const uint16_t *utf16_string,
     size_t utf16_string_length,
     libcerror_error_t **error )
{
	libbde_internal_volume_t *internal_volume = NULL;
	static char *function                     = "libbde_volume_set_utf16_recovery_password";

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( libbde_utf16_recovery_password_calculate_hash(
	     utf16_string,
	     utf16_string_length,
	     internal_volume->io_handle->recovery_password_hash,
	     32,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set recovery password hash.",
		 function );

		return( -1 );
	}
	internal_volume->io_handle->recovery_password_is_set = 1;

	return( 1 );
}

/* Sets the startup key from a .BEK file
 * This function needs to be used before one of the open functions
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_read_startup_key(
     libbde_volume_t *volume,
     const char *filename,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle = NULL;
	static char *function            = "libbde_volume_read_startup_key";
	size_t filename_length           = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	filename_length = libcstring_narrow_string_length(
	                   filename );

	if( filename_length == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		goto on_error;
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
	if( libbfio_file_set_name(
	     file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		goto on_error;
	}
	if( libbde_volume_read_startup_key_file_io_handle(
	     volume,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open startup key file: %s.",
		 function,
		 filename );

		goto on_error;
	}
	if( libbfio_handle_free(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free file IO handle.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
        return( -1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Sets the startup key from a .BEK file
 * This function needs to be used before one of the open functions
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_read_startup_key_wide(
     libbde_volume_t *volume,
     const wchar_t *filename,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle = NULL;
	static char *function            = "libbde_volume_read_startup_key_wide";
	size_t filename_length           = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	filename_length = libcstring_wide_string_length(
	                   filename );

	if( filename_length == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		goto on_error;
	}
	if( libbfio_file_initialize(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO handle.",
		 function );

		goto on_error;
	}
	if( libbfio_file_set_name_wide(
	     file_io_handle,
	     filename,
	     filename_length,
	     error ) != 1 )
	{
                libcerror_error_set(
                 error,
                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
                 "%s: unable to set filename in file IO handle.",
                 function );

		goto on_error;
	}
	if( libbde_volume_read_startup_key_file_io_handle(
	     volume,
	     file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open startup key file: %ls.",
		 function,
		 filename );

		goto on_error;
	}
	if( libbfio_handle_free(
	     &file_io_handle,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free file IO handle.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
        return( -1 );
}

#endif

/* Sets the startup key from a .BEK file using a Basic File IO (bfio) handle
 * This function needs to be used before one of the open functions
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_read_startup_key_file_io_handle(
     libbde_volume_t *volume,
     libbfio_handle_t *file_io_handle,
     libcerror_error_t **error )
{
	bde_metadata_header_v1_t file_header;

	libbde_internal_volume_t *internal_volume = NULL;
	uint8_t *metadata_entries_data            = NULL;
	static char *function                     = "libbde_volume_read_startup_key_wide";
	ssize_t read_count                        = 0;
	uint32_t metadata_size                    = 0;
	int file_io_handle_is_open                = 0;

	if( volume == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume.",
		 function );

		return( -1 );
	}
	internal_volume = (libbde_internal_volume_t *) volume;

	if( internal_volume->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid volume - missing IO handle.",
		 function );

		return( -1 );
	}
	if( internal_volume->file_io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - file IO handle already set.",
		 function );

		return( -1 );
	}
	if( internal_volume->external_key_metadata != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid volume - external key metadata already set.",
		 function );

		return( -1 );
	}
	if( file_io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file IO handle.",
		 function );

		return( -1 );
	}
	file_io_handle_is_open = libbfio_handle_is_open(
	                          file_io_handle,
	                          error );

	if( file_io_handle_is_open == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open file.",
		 function );

		return( -1 );
	}
	else if( file_io_handle_is_open == 0 )
	{
		if( libbfio_handle_open(
		     file_io_handle,
		     LIBBFIO_OPEN_READ,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_OPEN_FAILED,
			 "%s: unable to open file.",
			 function );

			return( -1 );
		}
	}
	if( libbfio_handle_seek_offset(
	     file_io_handle,
	     0,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek file header offset: 0.",
		 function );

		goto on_error;
	}
	if( libbde_metadata_initialize(
	     &( internal_volume->external_key_metadata ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create external key metadata.",
		 function );

		goto on_error;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading BitLocker External Key (BEK) metadata:\n" );
	}
#endif
	read_count = libbfio_handle_read_buffer(
	              file_io_handle,
	              (uint8_t *) &file_header,
	              sizeof( bde_metadata_header_v1_t ),
	              error );

	if( read_count != sizeof( bde_metadata_header_v1_t ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read file header.",
		 function );

		goto on_error;
	}
	read_count = libbde_metadata_read_header(
	              internal_volume->external_key_metadata,
	              (uint8_t *) &file_header,
	              sizeof( bde_metadata_header_v1_t ),
	              &metadata_size,
	              error );

	if( read_count == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read external key metadata header.",
		 function );

		goto on_error;
	}
	if( metadata_size < sizeof( bde_metadata_header_v1_t ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: metadata size value out of bounds.",
		 function );

		goto on_error;
	}
	metadata_size -= sizeof( bde_metadata_header_v1_t );

	metadata_entries_data = (uint8_t *) memory_allocate(
	                                     sizeof( uint8_t ) * metadata_size );

	if( metadata_entries_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to metadata entries data.",
		 function );

		goto on_error;
	}
	read_count = libbfio_handle_read_buffer(
	              file_io_handle,
	              metadata_entries_data,
	              (size_t) metadata_size,
	              error );

	if( read_count != (ssize_t) metadata_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read metadata entries data.",
		 function );

		goto on_error;
	}
	read_count = libbde_metadata_read_entries(
	              internal_volume->external_key_metadata,
	              metadata_entries_data,
	              (size_t) metadata_size,
	              NULL,
	              0,
	              error );

	if( read_count == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read external key metadata entries.",
		 function );

		goto on_error;
	}
	memory_free(
	 metadata_entries_data );

	metadata_entries_data = NULL;

	if( file_io_handle_is_open == 0 )
	{
		if( libbfio_handle_close(
		     file_io_handle,
		     error ) != 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_CLOSE_FAILED,
			 "%s: unable to close file.",
			 function );

			return( -1 );
		}
	}
	return( 1 );

on_error:
	if( metadata_entries_data != NULL )
	{
		memory_free(
		 metadata_entries_data );
	}
	if( internal_volume->external_key_metadata != NULL )
	{
		libbde_metadata_free(
		 &( internal_volume->external_key_metadata ),
		 NULL );
	}
	if( file_io_handle_is_open == 0 )
	{
		libbfio_handle_close(
		 file_io_handle,
		 error );
	}
	return( -1 );
}

