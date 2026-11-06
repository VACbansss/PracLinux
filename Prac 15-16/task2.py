#!/usr/bin/env python3
from bcc import BPF
from time import sleep

bpf_text = r"""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
#include <bcc/proto.h>
/*
* saddr, daddr IP-адреса 
* sport, dport TCP-порты
*/
struct conn_key_t {
    u32 saddr, daddr;
    u16 sport, dport;
};
BPF_HASH(conns, struct conn_key_t, u64); // ключ соединения + значение
BPF_HASH(tx_bytes, u32, u64); // ключ ifindex + счетчик байт

TRACEPOINT_PROBE(sock, inet_sock_set_state) {
    if (args->newstate != 1) return 0; // Отсекаем все, кроме TCP_ESTABLISHED
    
    struct conn_key_t key = {};
    
    // Копирование памяти через __builtin_memcpy т.к. saddr, daddr - массивы
    __builtin_memcpy(&key.saddr, args->saddr, sizeof(key.saddr));
    __builtin_memcpy(&key.daddr, args->daddr, sizeof(key.daddr));
    
    key.sport = args->sport; 
    key.dport = args->dport;
    //Кладем в карту значение времени и ключ
    u64 ts = bpf_ktime_get_ns();
    conns.update(&key, &ts);
    return 0;
}

TRACEPOINT_PROBE(net, net_dev_queue) {
    u32 ifindex = 1;
    u64 len = args->len;
    u64 *v = tx_bytes.lookup(&ifindex);
    //Если запись по ifindex уже есть, прибавляем длину пакета
    if (v) __sync_fetch_and_add(v, len);
    else tx_bytes.update(&ifindex, &len);
    return 0;
}
"""

b = BPF(text=bpf_text)
print("Трассировка сети...")
# Сбор статистики 20 секунд
try: 
    sleep(20)
except KeyboardInterrupt: 
    pass
# Считываем значение с карт conns и tx_bytes и выводим в консоль
print("\n=== Соединения (выборка) ===")
for k, v in list(b["conns"].items())[:10]:
    print(f"{k.saddr:#x}:{k.sport} -> {k.daddr:#x}:{k.dport}")

print("\n=== Отправлено байт по ifindex ===")
for k, v in b["tx_bytes"].items():
    print(f"ifindex {k.value}: {v.value} байт")
