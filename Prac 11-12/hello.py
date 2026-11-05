#!/usr/bin/python3  
from bcc import BPF

//hello - функция выполняется при срабатывании kprobe
//bpf_trace_printk - встроенный helper ядра, пишущий строку в специальный trace_pipe
program = r"""
int hello(void *ctx) { 
    bpf_trace_printk("Hello World!");
    return 0;
}
"""
b = BPF(text=program) //Компиляция и загрузка кода 
syscall = b.get_syscall_fnname("execve") //Привязка к событию ядра
b.attach_kprobe(event=syscall, fn_name="hello")//Привязка функции к событию в ядре (когда процесс вызывает execve, ядро выполнит hello())

b.trace_print()
