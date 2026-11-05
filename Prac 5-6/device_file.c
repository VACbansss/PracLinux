#include "device_file.h"
#include <linux/fs.h>       /* file stuff */
#include <linux/kernel.h>   /* printk() */
#include <linux/errno.h>    /* error codes */
#include <linux/module.h>   /* THIS_MODULE */
#include <linux/cdev.h>     /* char device stuff */
#include <linux/uaccess.h>  /* copy_to_user() */

static const char g_s_Hello_World_string[] = "Hello world from kernel mode!\n\0";
static const ssize_t g_s_Hello_World_size = sizeof(g_s_Hello_World_string);

/*
 * Вызывается ядром при read() из файла устройства.
 * file_ptr указатель на структуру открытого файла;
 * user_buffer буфер в пространстве пользователя, куда нужно скопировать данные;
 * count максимальное число байт, которое пользователь хочет прочитать;
 * position указатель на текущую позицию (смещение) чтения в файле.
 * Возвращает число прочитанных байт, 0 при EOF или отрицательный код ошибки.
 */
static ssize_t device_file_read(
    struct file *file_ptr,
    char __user *user_buffer,
    size_t count,
    loff_t *position)
{
    printk(KERN_NOTICE "Simple-driver: Device file is read at offset = %i, read bytes count = %u\n",
           (int)*position, (unsigned int)count); // печать в журнале ядра, информация о чтении: смещении и запрошенное количество байт

    
    if (*position >= g_s_Hello_World_size) //При выходе за границы текста возвращает 0
        return 0; 

    
    if (*position + count > g_s_Hello_World_size) //Уменьшение байт до остатка, ориентир g_s_Hello_World_size
        count = g_s_Hello_World_size - *position;

    
    if (copy_to_user(user_buffer, g_s_Hello_World_string + *position, count) != 0) // Копирует данные из пространства ядра в пространство пользователя, при ошибке возвращает -EFAULT
        return -EFAULT; 

    
    *position += count; //Сдвиг позиции чтения файла

    return count;
}

/*Таблица функций, вызываемое ядром для операций над устройством
* .owner = THIS_MODULE владелец модуля, говорит об использовании модуля, если модуль используется нельзя выгрузить
* .read = device_file_read при чтении вызывается функция device_file_read
*/
static struct file_operations simple_driver_fops =
{
    .owner = THIS_MODULE,
    .read = device_file_read,
};
//Регистрация драйвера
static int device_file_major_number = 0; // = 0 показывает что устройство ещё не зарегистрированно 
static const char device_name[] = "Simple-driver"; // имя регистрируемого устройства

//Функция регистрации в ядре, вызывается один раз при загрузке модуля
int register_device(void)
{
    int result = 0;

    printk(KERN_NOTICE "Simple-driver: register_device() is called.\n");
    // Функция посимвольной регистрации в ядре. Аргументы: 0 - функция сама находит незанятый номер ядром, device_name - имя драйвера, &simple_driver_fops - указатель на таблицу операций
    result = register_chrdev(0, device_name, &simple_driver_fops);
    if (result < 0) { //Проверка регистрации, если result не ноль при регистрации произошла ошибка
        printk(KERN_WARNING "Simple-driver: can't register character device with error code = %i\n", result);
        return result;
    }

    device_file_major_number = result;
    //Сообщение о успешно зарегистрированном драйвере
    printk(KERN_NOTICE "Simple-driver: registered character device with major number = %i and minor numbers 0...255\n",
           device_file_major_number);

    return 0;
}
//Функция снятия регистрации
void unregister_device(void)
{
    printk(KERN_NOTICE "Simple-driver: unregister_device() is called\n");

    if (device_file_major_number != 0) { //Если мажорное число не нулевое - снимается с регистрации
        unregister_chrdev(device_file_major_number, device_name);
        device_file_major_number = 0;
        printk(KERN_NOTICE "Simple-driver: character device unregistered\n");
    }
}
