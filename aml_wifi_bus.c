#include <linux/init.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "aml_wifi_bus.h"
#include "aml_sdio_common.h"
#include "aml_usb_common.h"
#include "aml_pci_common.h"

int aml_wifi_bus_insmod(void)
{
    int ret;

    ret = aml_wifi_usb_insmod();
    if (ret) {
        printk("aml usb bus insmod fail\n");
    }
    ret = aml_wifi_sdio_insmod();
    if (ret) {
        printk("aml sdio bus insmod fail\n");
    }
    ret = aml_wifi_pci_insmod();
    if (ret) {
        printk("aml pcie bus insmod fail\n");
    }
    return 0;
}

void aml_wifi_bus_rmmod(void)
{
    aml_wifi_usb_rmmod();
    aml_wifi_sdio_rmmod();
    aml_wifi_pci_rmmod();
    printk("rmmod aml wifi comm");
}
module_init(aml_wifi_bus_insmod);
module_exit(aml_wifi_bus_rmmod);

MODULE_LICENSE("GPL");
