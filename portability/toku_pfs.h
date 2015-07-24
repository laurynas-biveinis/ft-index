#pragma once

#include <stdio.h> // FILE

enum class toku_instr_object_type { mutex, thread, file };

struct PSI_file;

struct TOKU_FILE
{
    /** The real file. */
    FILE *file;
    struct PSI_file *psi_key;
};

class toku_instr_key;

class toku_instr_probe_empty
{
public:
    explicit toku_instr_probe_empty(UU(const toku_instr_key &key)) { }

    void start_with_source_location(UU(const char *src_file), UU(int src_line))
    { }

    void stop() { }
};

#define TOKU_PROBE_START(p) p->start_with_source_location(__FILE__, __LINE__)

extern toku_instr_key toku_uninstrumented;

#ifndef MYSQL_TOKUDB_ENGINE

#include <pthread.h>

class toku_instr_key
{
public:
    toku_instr_key(UU(toku_instr_object_type type), UU(const char *group),
                   UU(const char *name)) { }

    explicit toku_instr_key(UU(pfs_key_t key_id)) { }

};

typedef toku_instr_probe_empty toku_instr_probe;

enum class toku_instr_file_op { fopen, create, open };

struct PSI_file { };
struct PSI_mutex { };

struct toku_io_instrumentation { };

inline
int toku_pthread_create(UU(const toku_instr_key &key), pthread_t *thread,
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
                                UU(const toku_instr_key &key),
                                UU(toku_instr_file_op op),
                                UU(const char *name), UU(const char *src_file),
                                UU(int src_line))
{
}

inline
void toku_instr_fopen_end(UU(toku_io_instrumentation &io_instr),
                          UU(TOKU_FILE &file))
{
}

struct toku_mutex_t;

struct toku_mutex_instrumentation { };

inline
PSI_mutex* toku_instr_mutex_init(UU(const toku_instr_key &key),
                                 UU(const toku_mutex_t &mutex))
{
    return nullptr;
}

inline
void toku_instr_mutex_destroy(UU(PSI_mutex* &mutex_instr))
{
}

inline
void toku_instr_mutex_lock_start(UU(toku_mutex_instrumentation &mutex_instr),
                                 UU(toku_mutex_t &mutex),
                                 UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_mutex_trylock_start(UU(toku_mutex_instrumentation &mutex_instr),
                                    UU(toku_mutex_t &mutex),
                                    UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_mutex_lock_end(UU(toku_mutex_instrumentation &mutex_instr),
                               UU(int pthread_mutex_lock_result))
{
}

inline
void toku_instr_mutex_unlock(UU(PSI_mutex *mutex_instr))
{
}

#else // MYSQL_TOKUDB_ENGINE

#include <toku_mysql.h>
// # include <my_pthread.h>
// # include <mysql/plugin.h>

#endif // MYSQL_TOKUDB_ENGINE

extern toku_instr_key toku_uninstrumented;

extern toku_instr_probe *toku_instr_probe_1;

//threads
extern toku_instr_key *extractor_thread_key;
extern toku_instr_key *fractal_thread_key;
extern toku_instr_key *io_thread_key;
extern toku_instr_key *eviction_thread_key;
extern toku_instr_key *kibbutz_thread_key;
extern toku_instr_key *minicron_thread_key;
extern toku_instr_key *tp_internal_thread_key;

// Files
extern toku_instr_key *tokudb_file_data_key;
extern toku_instr_key *tokudb_file_load_key;
extern toku_instr_key *tokudb_file_tmp_key;
extern toku_instr_key *tokudb_file_log_key;

// Mutexes
extern toku_instr_key *kibbutz_mutex_key;
extern toku_instr_key *minicron_p_mutex_key;
extern toku_instr_key *queue_result_mutex_key;
extern toku_instr_key *tpool_lock_mutex_key;
extern toku_instr_key *workset_lock_mutex_key;
extern toku_instr_key *bjm_jobs_lock_mutex_key;
extern toku_instr_key *log_internal_lock_mutex_key;
extern toku_instr_key *cachetable_ev_thread_lock_mutex_key;
extern toku_instr_key *checkpoint_safe_mutex_key;
extern toku_instr_key *ft_ref_lock_mutex_key;
extern toku_instr_key *loader_error_mutex_key;
extern toku_instr_key *bfs_mutex_key;
extern toku_instr_key *loader_bl_mutex_key;
extern toku_instr_key *loader_fi_lock_mutex_key;
extern toku_instr_key *loader_out_mutex_key;
extern toku_instr_key *result_output_condition_lock_mutex_key;
extern toku_instr_key *block_allocator_trace_lock_mutex_key;
extern toku_instr_key *block_table_mutex_key;
extern toku_instr_key *rollback_log_node_cache_mutex_key;
extern toku_instr_key *txn_lock_mutex_key;
extern toku_instr_key *txn_state_lock_mutex_key;
extern toku_instr_key *txn_child_manager_mutex_key;
extern toku_instr_key *txn_manager_lock_mutex_key;
extern toku_instr_key *treenode_mutex_key;
extern toku_instr_key *manager_mutex_key;
extern toku_instr_key *manager_escalation_mutex_key;
extern toku_instr_key *manager_escalator_mutex_key;
extern toku_instr_key *db_txn_struct_i_txn_mutex_key;
extern toku_instr_key *indexer_i_indexer_lock_mutex_key;
extern toku_instr_key *indexer_i_indexer_estimate_lock_mutex_key;
extern toku_instr_key *locktree_request_info_mutex_key;

// UNCONVERTED PART BELOW

#define PFS_NOT_INSTRUMENTED           0xFFFFFFFF

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

