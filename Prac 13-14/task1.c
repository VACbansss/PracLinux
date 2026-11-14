#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

/*
* pid  — номер процесса
* comm — имя задачи
*/
struct key_t {
    u32 pid;
    char comm[16];
};

/*
* alloc_bytes — суммарный объём памяти
* free_bytes  — суммарный объём освобождённой памяти
* alloc_cnt   — количество вызовов kmalloc
* free_cnt    — количество вызовов kfree
*/
struct val_t {
    u64 alloc_bytes;
    u64 free_bytes;
    u64 alloc_cnt;
    u64 free_cnt;
};

BPF_HASH(stats, struct key_t, struct val_t);

// Tracepoint для выделения памяти
TRACEPOINT_PROBE(kmem, kmalloc) {
    struct key_t key = {};
    key.pid = bpf_get_current_pid_tgid() >> 32; // получаем PID
    bpf_get_current_comm(&key.comm, sizeof(key.comm)); // копирование имени задачи

    struct val_t *val = stats.lookup_or_try_init(&key, &(struct val_t){}); // находим или создаём запись о процессе по ключу
    if (val) { // обновляем значения счётчиков для процесса
        val->alloc_cnt++;
        val->alloc_bytes += args->bytes_alloc;
    }
    return 0;
}

// Tracepoint для освобождения памяти
TRACEPOINT_PROBE(kmem, kfree) {
    struct key_t key = {};
    key.pid = bpf_get_current_pid_tgid() >> 32; // получаем PID
    bpf_get_current_comm(&key.comm, sizeof(key.comm)); // копирование имени задачи

    struct val_t *val = stats.lookup(&key); // ищем существующую запись о процессе по ключу
    if (val) { // обновляем значения счётчиков для процесса
        val->free_cnt++;
    }
    return 0;
}