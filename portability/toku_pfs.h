#pragma once

#include <stdio.h> // FILE

// TODO: this define disappears after the source is fully converted to
// toku_uninstrumented
#define PFS_NOT_INSTRUMENTED           0xFFFFFFFF

enum class toku_instr_object_type { mutex, rwlock, cond, thread, file };

struct PSI_file;

struct TOKU_FILE
{
    /** The real file. */
    FILE *file;
    struct PSI_file * key;
};

struct PSI_mutex;

struct toku_mutex_t {
    pthread_mutex_t pmutex;
#if TOKU_PTHREAD_DEBUG
    pthread_t owner; // = pthread_self(); // for debugging
    bool locked;
    bool valid;
#endif

    struct PSI_mutex* psi_mutex;      /* The performance schema instrumentation hook */
#if TOKU_PTHREAD_DEBUG
    pfs_key_t instr_key_id;
#endif
};

class toku_instr_key;

// TODO: break this include loop
#include <pthread.h>
typedef pthread_mutexattr_t toku_pthread_mutexattr_t;

inline
void toku_mutex_init(const toku_instr_key &key, toku_mutex_t *mutex,
                     const toku_pthread_mutexattr_t *attr);

inline
void toku_mutex_destroy(toku_mutex_t *mutex);

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

enum class toku_instr_file_op { file_stream_open, file_create, file_open,
        file_delete, file_read, file_write, file_sync, file_stream_close,
        file_close };

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
void toku_instr_file_stream_open_end(UU(toku_io_instrumentation &io_instr),
                          UU(TOKU_FILE &file))
{
}

inline
void toku_instr_file_open_end(UU(toku_io_instrumentation &io_instr), UU(int fd))
{
}

inline
void toku_instr_file_name_close_begin(UU(toku_io_instrumentation &io_instr),
                                UU(const toku_instr_key &key),
                                UU(toku_instr_file_op op),
                                UU(const char *name), UU(const char *src_file),
                                UU(int src_line))
{
}

inline
void toku_instr_file_stream_close_begin(UU(toku_io_instrumentation &io_instr),
                                UU(toku_instr_file_op op), UU(TOKU_FILE &file), 
                                UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_file_fd_close_begin(UU(toku_io_instrumentation &io_instr),
                                UU(toku_instr_file_op op), UU(int fd), 
                                UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_file_close_end(UU(toku_io_instrumentation &io_instr), UU(int result))
{
}

inline
void toku_instr_file_io_begin(UU(toku_io_instrumentation &io_instr),
                                UU(toku_instr_file_op op), UU(int fd), 
                                UU(unsigned int count),
                                UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_file_name_io_begin(UU(toku_io_instrumentation &io_instr),
                                UU(const toku_instr_key &key),
                                UU(toku_instr_file_op op),
                                UU(const char *name),
                                UU(unsigned int count),
                                UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_file_stream_io_begin(UU(toku_io_instrumentation &io_instr),
                                UU(toku_instr_file_op op), UU(TOKU_FILE &file), 
                                UU(unsigned int count),
                                UU(const char *src_file), UU(int src_line))
{
}

inline
void toku_instr_file_io_end(UU(toku_io_instrumentation &io_instr), 
                            UU(unsigned int count))
{
}


struct toku_mutex_t;

struct toku_mutex_instrumentation { };

inline
PSI_mutex* toku_instr_mutex_init(UU(const toku_instr_key &key),
                                 UU(toku_mutex_t &mutex))
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

