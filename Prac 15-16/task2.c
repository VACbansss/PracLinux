#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
#include <bcc/proto.h>

/*
* saddr, daddr — IP-адреса
* sport, dport — TCP-порты
*/
struct conn_key_t {
    u32 saddr, daddr;
    u16 sport, dport;
};

BPF_HASH(conns, struct conn_key_t, u64); // ключ соединения + значение
BPF_HASH(tx_bytes, u32, u64);            // ключ ifindex + счётчик байт

TRACEPOINT_PROBE(sock, inet_sock_set_state) {
    if (args->newstate != 1) return 0; // Отсекаем всё, кроме TCP_ESTABLISHED

    struct conn_key_t key = {};

    // Копирование памяти через __builtin_memcpy, т.к. saddr, daddr — массивы
    __builtin_memcpy(&key.saddr, args->saddr, sizeof(key.saddr));
    __builtin_memcpy(&key.daddr, args->daddr, sizeof(key.daddr));

    key.sport = args->sport;
    key.dport = args->dport;

    // Кладём в карту значение времени и ключ
    u64 ts = bpf_ktime_get_ns();
    conns.update(&key, &ts);
    return 0;
}

TRACEPOINT_PROBE(net, net_dev_queue) {
    u32 ifindex = 1;
    u64 len = args->len;
    u64 *v = tx_bytes.lookup(&ifindex);
    // Если запись по ifindex уже есть, прибавляем длину пакета
    if (v) __sync_fetch_and_add(v, len);
    else tx_bytes.update(&ifindex, &len);
    return 0;
}