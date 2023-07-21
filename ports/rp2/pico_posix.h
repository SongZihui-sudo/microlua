#ifndef posix_h
#define posix_h

#include "lfs.h"

FILE* toFILE( lfs_file_t* fp );

lfs_file_t* tolfs_FILE( FILE* fp );

FILE* lfs_fopen( const char* filename, const char* mode );

int lfs_fclose( FILE* stream );

size_t lfs_fread( void* ptr, size_t size, size_t nitems, FILE* stream );

size_t lfs_fwrite( void* ptr, size_t size, size_t nitems, FILE* stream );

void lfs_funlockfile( FILE* file );

void lfs_flockfile( FILE* file );

#define funlockfile lfs_funlockfile
#define flockfile lfs_flockfile
#define popen lfs_fopen
#define pclose lfs_fclose
#define fread lfs_fread
#define fwrite lfs_fwrite

#endif
