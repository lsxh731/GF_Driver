#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5602ea1a, "module_layout" },
	{ 0x16cfe0bf, "platform_driver_unregister" },
	{ 0x8d090a07, "platform_driver_register" },
	{ 0xf7759a51, "pwm_enable" },
	{ 0xc0f68f29, "pwm_config" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xe5a8aba1, "pwm_request" },
	{ 0x2db6ffff, "mxc_iomux_v3_setup_pad" },
	{ 0xfc329187, "misc_register" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xa5832348, "pwm_free" },
	{ 0xba3ff362, "pwm_disable" },
	{ 0xfe990052, "gpio_free" },
	{ 0x8b66baa4, "misc_deregister" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x27e1a049, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

