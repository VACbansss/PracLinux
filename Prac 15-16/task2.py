#!/usr/bin/env python3
from bcc import BPF
from time import sleep

bpf_text = r"""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
#include <bcc/proto.h>

struct conn_key_t {
    u32 saddr, daddr;
    u16 sport, dport;
};
BPF_HASH(conns, struct conn_key_t, u64);
BPF_HASH(tx_bytes, u32, u64);

TRACEPOINT_PROBE(sock, inet_sock_set_state) {
    // TCP_ESTABLISHED = 1. Убрали зависимость от net/tcp.h
    if (args->newstate != 1) return 0;
    
    struct conn_key_t key = {};
    
    // saddr/daddr в tracepoint — это массивы __u8[4]
    __builtin_memcpy(&key.saddr, args->saddr, sizeof(key.saddr));
    __builtin_memcpy(&key.daddr, args->daddr, sizeof(key.daddr));
    
    key.sport = args->sport; 
    key.dport = args->dport;
    
    u64 ts = bpf_ktime_get_ns();
    conns.update(&key, &ts);
    return 0;
}

TRACEPOINT_PROBE(net, net_dev_queue) {
    u32 ifindex = 1;  // упрощённо для lo
    u64 len = args->len;
    u64 *v = tx_bytes.lookup(&ifindex);
    if (v) __sync_fetch_and_add(v, len);
    else tx_bytes.update(&ifindex, &len);
    return 0;
}
"""

b = BPF(text=bpf_text)
print("Трассировка сети... Ctrl-C для итогов")
try: 
    sleep(20)
except KeyboardInterrupt: 
    pass

print("\n=== Соединения (выборка) ===")
for k, v in list(b["conns"].items())[:10]:
    print(f"{k.saddr:#x}:{k.sport} -> {k.daddr:#x}:{k.dport}")

print("\n=== Отправлено байт по ifindex ===")
for k, v in b["tx_bytes"].items():
    print(f"ifindex {k.value}: {v.value} байт")