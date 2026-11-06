#!/usr/bin/env python3
from bcc import BPF
from time import sleep

# Используем tracepoint kmem:kmalloc и kmem:kfree вместо kprobe
bpf_text = r"""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
/*
* pid номер процесса
* comm имя задачи
*/
struct key_t {
    u32 pid;
    char comm[16];
};
/*
* alloc_bytes суммарный объем памяти
* free_bytes суммарный объем освобожденной памяти
* alloc_cnt количество вызовов kmalloc
* free_cnt количество вызовов kfree
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
    bpf_get_current_comm(&key.comm, sizeof(key.comm)); // копирование имя задачи

    struct val_t *val = stats.lookup_or_try_init(&key, &(struct val_t){}); // находим или создаем запись о процессе по ключу
    if (val) { // обновляем значения счетчиков для процесса
        val->alloc_cnt++;
        val->alloc_bytes += args->bytes_alloc;
    }
    return 0;
}

// Tracepoint для освобождения памяти
TRACEPOINT_PROBE(kmem, kfree) {
    struct key_t key = {};
    key.pid = bpf_get_current_pid_tgid() >> 32; // получаем PID
    bpf_get_current_comm(&key.comm, sizeof(key.comm)); // копирование имя задачи

    struct val_t *val = stats.lookup(&key); // находим или создаем запись о процессе по ключу
    if (val) { // обновляем значения счетчиков для процесса
        val->free_cnt++;
    }
    return 0;
}
"""

b = BPF(text=bpf_text)
print("Трассировка памяти...")
# Сбор статистики 30 секунд
try:
    sleep(30)
except KeyboardInterrupt:
    pass
# Считываем ранее записанные в структуры данные о выделении и освобождении памяти для процессов
print("\n%-16s %-8s %-10s %-10s %-10s" %
      ("COMM", "PID", "ALLOC_KB", "FREE_CNT", "ALLOC_CNT"))
for k, v in b["stats"].items():
    print("%-16s %-8d %-10d %-10d %-10d" %
          (k.comm.decode(), k.pid, v.alloc_bytes // 1024,
           v.free_cnt, v.alloc_cnt))
