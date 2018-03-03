
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <asm/io.h>

/* Begin to build a generic kermel module for the Raspberry Pi3. */
/* Kenel source at: /home/rogerd/cdev/kernel/raspberry3/linux    */
/* Makefile: make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-    */

/* uname -r: 4.9.59-v7+ running Raspberry kernel version.        */

#define MY_MODULE_NAME "Achar"
#define ACHAR_MAJOR   1000
#define ACHAR_MINOR   0

/* Number of consecutive minor numbers to register.              */
#define ACHAR_DEVICES 1

#include <linux/semaphore.h>
static struct mutex init_sem;

#include <linux/platform_device.h>
static struct class *achar_class;

/* Raspberry Pi3 peripheral bus is 0x3F00_0000--0x3FFF_FFFF         */
#define SOC_REG_BASE 0x3F003000  /* Broadcom System Timer Registers */
#define SOC_REG_LEN  0x00000020  /* Broadcom System Timer Registers */



/* Structure to represent the character driver instance. */
struct achar_data_struct
{

        resource_size_t timer_start;    /* phys. address of the control registers */
        resource_size_t timer_end;      /* phys. address of the control registers */
        resource_size_t timer_size;
        void __iomem *timer_address;    /* virt. address of the control registers */

        struct device *dev;
        struct cdev cdev;               /* Char device structure */
        dev_t devt;

        void *private_data;
        bool is_open;
        struct mutex sem;

};

#include <linux/slab.h>
struct achar_data_struct * achar_data = NULL;

volatile void *Test_reg = (int*)SOC_REG_BASE;  /* Broadcom System Timer Register */

static const struct file_operations achar_fops = {
        .owner = THIS_MODULE,
        // .write = hwicap_write,
        // .read = hwicap_read,
        // .open = hwicap_open,
        // .release = hwicap_release,
};

/* Boilerplate Init, Exit functions.
 * Does not do platform probing in this module.
 * Probing would call request_mem_region() to setup memory map.
 */
static int __init Achar_init( void )
{
// volatile int *Test_reg = (int*)0x7E003000; /* Broadcom System Timer Register */
         int  test = 0;

        dev_t devt;
        int retval;

   printk( KERN_ALERT "Init "MY_MODULE_NAME" Module." );

        /* Allocate the driver instance data to hold data used by file_op functions. */
        achar_data = NULL;
        achar_data = kzalloc(sizeof(struct achar_data_struct), GFP_KERNEL);
        if (!achar_data) {
                printk( KERN_INFO "Couldn't allocate device data "MY_MODULE_NAME" result: %08X", retval );
                retval = -ENOMEM;
                return retval; /* kfree before every return of this function!!! */
        }

   // class = class_create(THIS_MODULE, "config");
        achar_class = class_create(THIS_MODULE, "achar_config");

   // mutex_init();
        mutex_init(&init_sem);

   // devt = MKDEV(MAJOR, MINOR);
   // retval = register_chrdev_region(devt, DEVICES, DRIVER_NAME);
        devt = MKDEV(ACHAR_MAJOR, ACHAR_MINOR);
        retval = register_chrdev_region(devt,
                                        ACHAR_DEVICES,
                                        MY_MODULE_NAME);

        if (retval < 0) {
                unregister_chrdev_region(devt, ACHAR_DEVICES);
                printk( KERN_ALERT "Init "MY_MODULE_NAME" register_chrdev_region failed." );
                return retval;
        }
        printk( KERN_INFO "register_chrdev_region "MY_MODULE_NAME" result: %08X", retval );

       // retval = platform_driver_register(struct &platform_driver);

   /* The following actions would occur when the platform driver is registered. */
   /* This driver will request a region and re-map the base registers here.     */
        if (!request_mem_region( SOC_REG_BASE, SOC_REG_LEN, MY_MODULE_NAME)) {
                // dev_err(dev, "Couldn't lock memory region at %Lx\n",
                //        (unsigned long long) SOC_REG_BASE);
                printk( KERN_ALERT "Init "MY_MODULE_NAME" request_mem_region failed." );
                release_mem_region( SOC_REG_BASE, SOC_REG_LEN);
                return( -EBUSY );
        }

        /* ioremap() returns void pointer, so cannot do pointer++ on it. */
        printk( KERN_INFO "IOremap "MY_MODULE_NAME"  Perif. %08X", (int)Test_reg );

        achar_data->timer_address = ioremap_nocache(SOC_REG_BASE, SOC_REG_LEN);
        Test_reg = achar_data->timer_address;

        if (!Test_reg) {
                // dev_err(dev, "ioremap() failed\n");
                printk( KERN_ALERT "Init "MY_MODULE_NAME" ioremap failed." );
                release_mem_region( SOC_REG_BASE, SOC_REG_LEN);
                retval = -ENOMEM;
                return( retval );
        }
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" Virtual %08X", (int)Test_reg );

   // Create and Add the driver cdev structure to assign a file_op structure. */
        cdev_init(&achar_data->cdev, &achar_fops);
        achar_data->cdev.owner = THIS_MODULE;
        retval = cdev_add(&achar_data->cdev, devt, 1);
        if (retval) {
                printk( KERN_INFO "cdev_add failed. "MY_MODULE_NAME" result: %08X", retval );
                // dev_err(dev, "cdev_add() failed\n");
                // goto failed3;
        }

        /* Register the driver with the platform driver. */
        //device_create(achar_class, dev, devt, NULL, "%s%d", DRIVER_NAME, id);

   /* Begin accessing the hardware register set using its Virtual addresses. */

        test = ioread32(Test_reg);
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" TimerCS %08X", (int)test );

        test = ioread32(Test_reg+4);
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" TimerLO %08X", (int)test );

        test = ioread32(Test_reg+8);
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" TimerHI %08X", (int)test );

        iowrite32(0x40000000, Test_reg+0x0C);
        iowrite32(0x10000000, Test_reg+0x10);
        iowrite32(0xC0000000, Test_reg+0x14);
        iowrite32(0x80000000, Test_reg+0x18);

        iowrite32(0x123456FF, Test_reg+0x00);
        test = ioread32(Test_reg+0x10);
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" TimerCS %08X", (int)test );
        test = ioread32(Test_reg+0x14);
        printk( KERN_INFO "IOremap "MY_MODULE_NAME" TimerCS %08X", (int)test );

   return( 0 );

}


static void __exit Achar_exit( void )
{
        dev_t devt;

        printk( KERN_INFO "Releasing "MY_MODULE_NAME" Module. %08X", ACHAR_MAJOR );

   //   class_destroy(class);
        class_destroy(achar_class);

   //   delete the cdev
        cdev_del(&achar_data->cdev);

   //   unmap the io-memory space
        iounmap( achar_data->timer_address );

   //   platform_driver_unregister(struct &platform_driver);
        /* The release region would occur when the platform driver un-registers. */
        release_mem_region( SOC_REG_BASE, SOC_REG_LEN);

   //   dev_t devt = MKDEV(MAJOR, MINOR);
        devt = MKDEV(ACHAR_MAJOR, ACHAR_MINOR);

   //   unregister_chrdev_region(devt, DEVICES);
        unregister_chrdev_region(devt, ACHAR_DEVICES);

        kfree(achar_data);

   printk( KERN_ALERT "Exiting "MY_MODULE_NAME" Module." );

   return;

}

module_init( Achar_init );
module_exit( Achar_exit );

MODULE_AUTHOR("Roger Dickerson");
MODULE_DESCRIPTION("An introductory Character Driver, (c) 2018");
MODULE_LICENSE("GPL");


