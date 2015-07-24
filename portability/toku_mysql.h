// No include header guard as this file is meant to be included exactly once.
// A second include should mean something's wrong

#include <mysql/psi/mysql_file.h> // PSI_file
#include <mysql/psi/mysql_thread.h> // PSI_mutex

#ifndef HAVE_PSI_MUTEX_INTERFACE
#error HAVE_PSI_MUTEX_INTERFACE required
#endif
#ifndef HAVE_PSI_THREAD_INTERFACE
#error HAVE_PSI_THREAD_INTERFACE required
#endif

// Instrumentation keys

class toku_instr_key
{
private:
    pfs_key_t   m_id;
public:
    toku_instr_key(toku_instr_object_type type,
                   const char *group,
                   const char *name)
    {
        switch (type)
        {
        case mutex:
            PSI_mutex_info mutex_info{&m_id, name, 0};
            mysql_mutex_register(group, &mutex_info, 1);
            break;
        case thread:
            PSI_thread_info thread_info{&m_id, name, 0};
            mysql_thread_register(group, &thread_info, 1);
            break;
        case file:
            PSI_file_info file_info{&m_id, name, 0};
            mysql_file_register(group, &file_info, 1);
        }
    }

    explicit toku_instr_key(pfs_key_t key_id) : m_id(key_id) { }

    pfs_key_t id() const { return m_id; }
};

typedef toku_instr_probe_pfs toku_instr_probe;

// Thread instrumentation

inline
int toku_pthread_create(const toku_instr_key &key, pthread_t *thread,
                        const pthread_attr_t *attr,
                        void *(*start_routine)(void*), void *arg)
{
    return PSI_THREAD_CALL(spawn_thread)(key.id(), thread, attr, start_routine,
                                         arg);
}

inline
void toku_instr_delete_current_thread()
{
    PSI_THREAD_CALL(delete_current_thread)();
}

// I/O instrumentation

enum class toku_instr_file_op {
    fopen = PSI_FILE_STREAM_OPEN,
    create = PSI_FILE_CREATE,
    open = PSI_FILE_OPEN
};

struct toku_io_instrumentation
{
    PSI_file_locker *locker;
    PSI_file_locker_state state;

    toku_io_instrumentation() : locker(nullptr) { }
};

inline
void toku_instr_file_open_begin(toku_io_instrumentation &io_instr,
                                const toku_instr_key &key,
                                toku_instr_file_op op,
                                const char *name, const char *src_file,
                                int src_line)
{
    io_instr.locker = PSI_FILE_CALL(get_thread_file_name_locker)
        (io_instr.state, key.id(), op, name, &io_instr.locker);
    if (toku_likely(io_instr.locker)) {
        PSI_FILE_CALL(start_file_open_wait)(io_instr.locker,
                                            src_file, src_line);
    }
}

inline
void toku_instr_fopen_end(toku_io_instrumentation &io_instr, TOKU_FILE &file)
{
    if (toku_likely(io_instr.locker))
        file.key = PSI_FILE_CALL(end_file_open_wait)(io_instr.locker,
                                                     file.file);
}

// Mutex instrumentation

struct toku_mutex_instrumentation
{
    PSI_mutex_locker *locker;
    PSI_mutex_locker_state state;

    toku_io_instrumentation() : locker(nullptr) { }
};

inline
PSI_mutex* toku_instr_mutex_init(const toku_instr_key &key,
                                 const toku_mutex_t &mutex)
{
    return PSI_MUTEX_CALL(init_mutex)(key.id(), &mutex.pmutex);
#if TOKU_PTHREAD_DEBUG
    mutex.instr_key_id = key.id();
    if (key != PFS_NOT_INSTRUMENTED && mutex->psi_mutex == NULL )
        fprintf(stderr,"initing tokudb mutex: key: %d NULL\n", key);
#endif
}

inline
void toku_instr_mutex_destroy(PSI_mutex* &mutex_instr)
{
    if (mutex_instr != nullptr)
    {
        PSI_MUTEX_CALL(destroy_mutex)(mutex_instr);
        mutex_instr = nullptr;
    }
}

inline
void toku_instr_mutex_lock_start(toku_mutex_instrumentation &mutex_instr,
                                 toku_mutex_t &mutex,
                                 const char *src_file, int src_line)
{
    if (mutex->psi_mutex)
    {
        mutex_instr.locker = PSI_MUTEX_CALL(start_mutex_wait)
            (&state, mutex.psi_mutex, PSI_MUTEX_LOCK, src_file, src_line);
#if TOKU_PTHREAD_DEBUG
        if (!mutex_instr.locker)
            fprintf(stderr, "Can't start timer for mutex key: %d\n",
                    mutex->instr_key_id);
#endif
    }
}

inline
void toku_instr_mutex_trylock_start(toku_mutex_instrumentation &mutex_instr,
                                    toku_mutex_t &mutex,
                                    const char *src_file, int src_line)
{
    if (mutex->psi_mutex)
        {
            mutex_instr.locker = PSI_MUTEX_CALL(start_mutex_wait)
                (&state, mutex.psi_mutex, PSI_MUTEX_TRYLOCK,
                 src_file, src_line);
        }
}

inline
void toku_instr_mutex_lock_end(toku_mutex_instrumentation &mutex_instr,
                               int pthread_mutex_lock_result)
{
    if (mutex_instr.locker)
        PSI_MUTEX_CALL(end_mutex_wait)(locker, pthread_mutex_lock_result);
}

inline
void toku_instr_mutex_unlock(PSI_mutex *mutex_instr)
{
    if (mutex_instr)
        PSI_MUTEX_CALL(unlock_mutex)(mutex_instr);
}

// Instrumentation probes

class toku_instr_probe_pfs
{
 private:
    toku_mutex_t mutex;
    toku_mutex_instrumentation mutex_instr;
 public:
    explicit toku_instr_probe_pfs(const toku_instr_key &key)
    {
        toku_mutex_init(key.id(), &mutex, nullptr);
    }

    ~toku_instr_probe_pfs()
    {
        toku_mutex_destroy(&mutex);
    }

    void start_with_source_location(const char *src_file, int src_line)
    {
        mutex_instr.locker = nullptr;
        toku_instr_mutex_lock_start(mutex_instr, mutex, src_file, src_line);
    }

    void stop()
    {
        toku_instr_mutex_lock_end(mutex_instr, 0);
    }
};
