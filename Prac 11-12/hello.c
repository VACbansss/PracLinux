// hello - функция выполняется при срабатывании kprobe
// bpf_trace_printk - встроенный helper ядра, пишущий строку в trace_pipe

int hello(void *ctx) {
    bpf_trace_printk("Hello World!");
    return 0;
}