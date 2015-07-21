#pragma once

#include <stdio.h> // FILE

struct PSI_file;

struct TOKU_FILE
{
    /** The real file. */
    FILE *file;
    struct PSI_file *psi_key;
};

#ifndef MYSQL_TOKUDB_ENGINE

#include <pthread.h>

enum toku_instr_file_op { toku_instr_fopen, toku_instr_create,
                          toku_instr_open };

struct PSI_file { };

struct toku_io_instrumentation { };

inline
int toku_pthread_create(UU(pfs_key_t key), pthread_t *thread,
                        const pthread_attr_t *attr,
                        void *(*start_routine)(void*), void *arg)
{
    return pthread_create(thread, attr, start_routine, arg);
}

inline
void toku_instr_delete_current_thread()
{
}

// Instrument file creation, opening, closing, and renaming
inline
void toku_instr_file_open_begin(UU(toku_io_instrumentation &io_instr),
                                UU(pfs_key_t key), UU(toku_instr_file_op op),
                                UU(const char *name), UU(const char *src_file),
                                UU(int src_line))
{
}

inline
void toku_instr_fopen_end(UU(toku_io_instrumentation &io_instr),
                          UU(TOKU_FILE &file))
{
}

#else // MYSQL_TOKUDB_ENGINE

#include <toku_mysql.h>
// # include <my_pthread.h>
// # include <mysql/plugin.h>

#endif // MYSQL_TOKUDB_ENGINE

// UNCONVERTED PART BELOW

#define PFS_NOT_INSTRUMENTED           0xFFFFFFFF

extern pfs_key_t  tokudb_file_data_key;

/* Following four macros are instumentations to register
various file I/O operations with performance schema.
2) register_pfs_file_io_begin() and register_pfs_file_io_end() are
used to register actual file read, write and flush
3) register_pfs_file_close_begin() and register_pfs_file_close_end()
are used to register file deletion operations*/
# define register_pfs_file_open_end(locker, fd)			\
do {									\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(end_file_open_wait_and_bind_to_descriptor)(\
			locker, fd);					\
	}								\
} while (0)

# define register_pfs_file_name_close_begin(state, locker, key, op,   	\
                                              name, src_file,    	\
                                              src_line)			\
do {									\
        locker = PSI_FILE_CALL(get_thread_file_name_locker)(            \
                        state, key, op, name, &locker);                 \
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_close_wait)(			\
			locker, src_file, src_line);			\
	}								\
} while (0)


# define register_pfs_file_stream_close_begin(state, locker, op,   	\
                                              file_stream, src_file,    \
                                              src_line)			\
do {									\
	locker = PSI_FILE_CALL(get_thread_file_stream_locker)(		\
		state, file_stream, op);				\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_close_wait)(			\
			locker, src_file, src_line);			\
	}								\
} while (0)

# define register_pfs_file_fd_close_begin(state, locker, op, fd,	\
				      src_file, src_line)		\
do {									\
	locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(	\
		state, fd, op);						\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_close_wait)(			\
			locker, src_file, src_line);			\
	}								\
} while (0)


# define register_pfs_file_close_end(locker, result)			\
do {									\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(end_file_close_wait)(			\
			locker, result);				\
	}								\
} while (0)

# define register_pfs_file_io_begin(state, locker, file, count, op,	\
				    src_file, src_line)			\
do {									\
	locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(	\
		state, file, op);					\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_wait)(				\
			locker, count, src_file, src_line);		\
	}								\
} while (0)

# define register_pfs_file_name_io_begin(state, locker, key, name, count, op,\
				    src_file, src_line)			\
do {									\
        locker = PSI_FILE_CALL(get_thread_file_name_locker)(            \
                        state, key, op, name, &locker);                 \
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_wait)(				\
			locker, count, src_file, src_line);		\
	}								\
} while (0)


# define register_pfs_file_stream_io_begin(state, locker, file_stream, count,	\
                                           op, src_file, src_line)	\
do {									\
        locker = PSI_FILE_CALL(get_thread_file_stream_locker)(          \
                state, file_stream, op);                                \
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(start_file_wait)(				\
			locker, count, src_file, src_line);		\
	}								\
} while (0)


# define register_pfs_file_io_end(locker, count)			\
do {									\
	if (toku_likely(locker != NULL)) {				\
		PSI_FILE_CALL(end_file_wait)(locker, count);		\
	}								\
} while (0)

