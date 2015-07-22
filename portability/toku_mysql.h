// No include header guard as this file is meant to be included exactly once.
// A second include should mean something's wrong

#include <mysql/psi/mysql_file.h> // PSI_file

#ifndef HAVE_PSI_MUTEX_INTERFACE
#error HAVE_PSI_MUTEX_INTERFACE required
#endif
#ifndef HAVE_PSI_THREAD_INTERFACE
#error HAVE_PSI_THREAD_INTERFACE required
#endif

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

class toku_instr_probe_pfs
{
private:
    toku_mutex_t                mutex;
    PSI_mutex_locker            *locker;
    PSI_mutex_locker_state      state;
public:
    explicit toku_instr_probe_pfs(const toku_instr_key &key) : locker(NULL)
    {
        toku_mutex_init(key.id(), &mutex, NULL);
    }

    ~toku_instr_probe_pfs()
    {
        toku_mutex_destroy(&mutex);
    }

    void start_with_source_location(const char *src_file, int src_line)
    {
        locker = NULL;
        if (mutex->psi_mutex)
        {
            locker = PSI_MUTEX_CALL(start_mutex_wait)(&state, mutex->psi_mutex,
                                                      PSI_MUTEX_LOCK,
                                                      src_file, src_line);
        }
    }

    void stop()
    {
        if (mutex->psi_mutex && locker)
            PSI_MUTEX_CALL(end_mutex_wait)(locker, 0);
    }
};

typedef toku_instr_probe_pfs toku_instr_probe;

enum toku_instr_file_op {
    toku_instr_fopen = PSI_FILE_STREAM_OPEN,
    toku_instr_create = PSI_FILE_CREATE,
    toku_instr_open = PSI_FILE_OPEN
};

struct toku_io_instrumentation
{
    PSI_file_locker *locker;
    PSI_file_locker_state state;

    toku_io_instrumentation() : locker(NULL) { }
};

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
