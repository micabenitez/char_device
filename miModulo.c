#include <linux/module.h>
#include <linux/kernel.h>
int init_module(void)
{ /* Constructor */
  printk(KERN_INFO "UNGS : Driver registrado\n");
  return 0;
}
void cleanup_module(void)
{ /* Destructor */
  printk(KERN_INFO "UNGS : Driver desregistrado\n");
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("UNGS");
MODULE_DESCRIPTION("Un primer driver");
