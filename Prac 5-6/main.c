#include "device_file.h"
#include <linux/init.h>       /* module_init, module_exit */
#include <linux/module.h>     /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */

/*===============================================================================================*/
static int simple_driver_init(void)
{
    int result = 0;

    printk(KERN_NOTICE "Simple-driver: module init\n");

    result = register_device();
    if (result < 0) {
        printk(KERN_WARNING "Simple-driver: failed to register device, error = %d\n", result);
        return result;
    }

    printk(KERN_NOTICE "Simple-driver: module loaded successfully\n");
    return 0;
}

/*===============================================================================================*/
static void simple_driver_exit(void)
{
    printk(KERN_NOTICE "Simple-driver: module exit\n");
    unregister_device();
    printk(KERN_NOTICE "Simple-driver: module unloaded\n");
}

/*===============================================================================================*/
module_init(simple_driver_init);
module_exit(simple_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Simple Linux character device driver");