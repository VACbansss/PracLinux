#!/usr/bin/env python3
from bcc import BPF
from time import sleep

# Используем tracepoint kmem:kmalloc и kmem:kfree вместо kprobe
bpf_text = r"""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

struct key_t {
    u32 pid;
    char comm[16];
};

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
    key.pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    struct val_t *val = stats.lookup_or_try_init(&key, &(struct val_t){});
    if (val) {
        val->alloc_cnt++;
        val->alloc_bytes += args->bytes_alloc;
    }
    return 0;
}

// Tracepoint для освобождения памяти
TRACEPOINT_PROBE(kmem, kfree) {
    struct key_t key = {};
    key.pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    struct val_t *val = stats.lookup(&key);
    if (val) {
        val->free_cnt++;
        // В kfree нет поля с размером, поэтому free_bytes не считаем
        // (для корректного подсчёта нужно было бы хранить map ptr->size, но для демо упрощаем)
    }
    return 0;
}
"""

b = BPF(text=bpf_text)
print("Трассировка памяти (tracepoint kmem)... Ctrl-C для итогов")
try:
    sleep(30)
except KeyboardInterrupt:
    pass

print("\n%-16s %-8s %-10s %-10s %-10s" %
      ("COMM", "PID", "ALLOC_KB", "FREE_CNT", "ALLOC_CNT"))
for k, v in b["stats"].items():
    print("%-16s %-8d %-10d %-10d %-10d" %
          (k.comm.decode(), k.pid, v.alloc_bytes // 1024,
           v.free_cnt, v.alloc_cnt))