// No include header guard as this file is meant to be included exactly once.
// A second include should mean something's wrong

#include <mysql/psi/mysql_file.h> // PSI_file

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

#ifdef HAVE_PSI_THREAD_INTERFACE

inline
int toku_pthread_create(pfs_key_t key, pthread_t *thread,
                        const pthread_attr_t *attr,
                        void *(*start_routine)(void*), void *arg)
{
    return PSI_THREAD_CALL(spawn_thread)(key, thread, attr, start_routine,
        arg);
}

inline
void toku_instr_delete_current_thread()
{
    PSI_THREAD_CALL(delete_current_thread)();
}

#endif // HAVE_PSI_THREAD_INTERFACE

inline
void toku_instr_file_open_begin(toku_io_instrumentation &io_instr,
                                pfs_key_t key, toku_instr_file_op op,
                                const char *name, const char *src_file,
                                int src_line)
{
    io_instr.locker = PSI_FILE_CALL(get_thread_file_name_locker)
        (io_instr.state, key, op, name, &io_instr.locker);
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
