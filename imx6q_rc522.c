#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <asm/uaccess.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
//#include <mach/soc.h>
//#include <mach/platform.h>
//#include <plat/gpio-cfg.h>
#include <linux/delay.h>

/*
 * This supports access to SPI devices using normal userspace I/O calls.
 * Note that while traditional UNIX/POSIX I/O semantics are half duplex,
 * and often mask message boundaries, full SPI support requires full duplex
 * transfers.  There are several kinds of internal message boundaries to
 * handle chipselect management and other protocol options.
 *
 * SPI has a character major number assigned.  We allocate minor numbers
 * dynamically using a bitmask.  You must use hotplug tools, such as udev
 * (or mdev with busybox) to create and destroy the /dev/spidevB.C device
 * nodes, since there is no fixed association of minor numbers with any
 * particular SPI bus or device.
 */
#define SPIDEV_MAJOR			153	/* assigned */
#define N_SPI_MINORS			32	/* ... up to 256 */

#define DEVICE_NAME				"ad760601"
#define DRIVER_NAME             "ad760601"

static DECLARE_BITMAP(minors, N_SPI_MINORS);


/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for some settings can cause *lots* of trouble for other
 * devices on a shared bus:
 *
 *  - CS_HIGH ... this device will be active when it shouldn't be
 *  - 3WIRE ... when active, it won't behave as it should
 *  - NO_CS ... there will be no explicit message boundaries; this
 *	is completely incompatible with the shared bus model
 *  - READY ... transfers may proceed when they shouldn't.
 *
 * REVISIT should changing those flags be privileged?
 */
#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY)

struct ad760601_data {
	dev_t			devt;
	spinlock_t		spi_lock;
	struct spi_device	*spi;
	struct list_head	device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	unsigned		users;
	u8			*buffer;
};

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 4096;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*-------------------------------------------------------------------------*/

#define AD760601_EIMD20_RESET	IMX_GPIO_NR(3, 20)

void ad760601_reset()
{
	//printk("************************ %s\n", __FUNCTION__);
	if(gpio_request(AD760601_EIMD20_RESET, "ad760601_RESET"))
                pr_err("failed to request GPK1_0 for ad760601 reset control\n");

	gpio_direction_output(AD760601_EIMD20_RESET, 0);

        mdelay(5);

        gpio_set_value(AD760601_EIMD20_RESET, 1);
        gpio_free(AD760601_EIMD20_RESET);
}

/*
 * We can't use the standard synchronous wrappers for file I/O; we
 * need to protect against async removal of the underlying spi_device.
 */
static void ad760601_complete(void *arg)
{
	complete(arg);
}

static ssize_t
ad760601_sync(struct ad760601_data *ad760601, struct spi_message *message)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	message->complete = ad760601_complete;
	message->context = &done;

	spin_lock_irq(&ad760601->spi_lock);
	if (ad760601->spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_async(ad760601->spi, message);
	spin_unlock_irq(&ad760601->spi_lock);

	if (status == 0) {
		wait_for_completion(&done);
		status = message->status;
		if (status == 0)
			status = message->actual_length;
	}
	return status;
}

static inline ssize_t
ad760601_sync_write(struct ad760601_data *ad760601, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= ad760601->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return ad760601_sync(ad760601, &m);
}

static inline ssize_t
ad760601_sync_read(struct ad760601_data *ad760601, size_t len)
{
	struct spi_transfer	t = {
			.rx_buf		= ad760601->buffer,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return ad760601_sync(ad760601, &m);
}

/*-------------------------------------------------------------------------*/

/* Read-only message with current device setup */
static ssize_t
ad760601_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct ad760601_data	*ad760601;
	ssize_t			status = 0;

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	ad760601 = filp->private_data;

	mutex_lock(&ad760601->buf_lock);
	status = ad760601_sync_read(ad760601, count);
	if (status > 0) {
		unsigned long	missing;

		missing = copy_to_user(buf, ad760601->buffer, status);
		if (missing == status)
			status = -EFAULT;
		else
			status = status - missing;
	}
	mutex_unlock(&ad760601->buf_lock);

	return status;
}

/* Write-only message with current device setup */
static ssize_t
ad760601_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct ad760601_data	*ad760601;
	ssize_t			status = 0;
	unsigned long		missing;

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz)
		return -EMSGSIZE;

	ad760601 = filp->private_data;

	mutex_lock(&ad760601->buf_lock);
	missing = copy_from_user(ad760601->buffer, buf, count);
	if (missing == 0) {
		status = ad760601_sync_write(ad760601, count);
	} else
		status = -EFAULT;
	mutex_unlock(&ad760601->buf_lock);

	return status;
}

static int ad760601_message(struct ad760601_data *ad760601,
		struct spi_ioc_transfer *u_xfers, unsigned n_xfers)
{
	struct spi_message	msg;
	struct spi_transfer	*k_xfers;
	struct spi_transfer	*k_tmp;
	struct spi_ioc_transfer *u_tmp;
	unsigned		n, total;
	u8			*buf;
	int			status = -EFAULT;

	spi_message_init(&msg);
	k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
	if (k_xfers == NULL)
		return -ENOMEM;

	/* Construct spi_message, copying any tx data to bounce buffer.
	 * We walk the array of user-provided transfers, using each one
	 * to initialize a kernel version of the same transfer.
	 */
	buf = ad760601->buffer;
	total = 0;
	for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;
			n;
			n--, k_tmp++, u_tmp++) {
		k_tmp->len = u_tmp->len;

		total += k_tmp->len;
		if (total > bufsiz) {
			status = -EMSGSIZE;
			goto done;
		}

		if (u_tmp->rx_buf) {
			k_tmp->rx_buf = buf;
			if (!access_ok(VERIFY_WRITE, (u8 __user *)
						(uintptr_t) u_tmp->rx_buf,
						u_tmp->len))
				goto done;
		}
		if (u_tmp->tx_buf) {
			k_tmp->tx_buf = buf;
			if (copy_from_user(buf, (const u8 __user *)
						(uintptr_t) u_tmp->tx_buf,
					u_tmp->len))
				goto done;
		}
		buf += k_tmp->len;

		k_tmp->cs_change = !!u_tmp->cs_change;
		k_tmp->bits_per_word = u_tmp->bits_per_word;
		k_tmp->delay_usecs = u_tmp->delay_usecs;
		k_tmp->speed_hz = u_tmp->speed_hz;
#ifdef VERBOSE
		dev_dbg(&ad760601->spi->dev,
			"  xfer len %zd %s%s%s%dbits %u usec %uHz\n",
			u_tmp->len,
			u_tmp->rx_buf ? "rx " : "",
			u_tmp->tx_buf ? "tx " : "",
			u_tmp->cs_change ? "cs " : "",
			u_tmp->bits_per_word ? : ad760601->spi->bits_per_word,
			u_tmp->delay_usecs,
			u_tmp->speed_hz ? : ad760601->spi->max_speed_hz);
#endif
		spi_message_add_tail(k_tmp, &msg);
	}

	status = ad760601_sync(ad760601, &msg);
	if (status < 0)
		goto done;

	/* copy any rx data out of bounce buffer */
	buf = ad760601->buffer;
	for (n = n_xfers, u_tmp = u_xfers; n; n--, u_tmp++) {
		if (u_tmp->rx_buf) {
			if (__copy_to_user((u8 __user *)
					(uintptr_t) u_tmp->rx_buf, buf,
					u_tmp->len)) {
				status = -EFAULT;
				goto done;
			}
		}
		buf += u_tmp->len;
	}
	status = total;

done:
	kfree(k_xfers);
	return status;
}

static long
ad760601_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int			err = 0;
	int			retval = 0;
	struct ad760601_data	*ad760601;
	struct spi_device	*spi;
	u32			tmp;
	unsigned		n_ioc;
	struct spi_ioc_transfer	*ioc;

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	/* guard against device removal before, or while,
	 * we issue this ioctl.
	 */
	ad760601 = filp->private_data;
	spin_lock_irq(&ad760601->spi_lock);
	spi = spi_dev_get(ad760601->spi);
	spin_unlock_irq(&ad760601->spi_lock);

	if (spi == NULL)
		return -ESHUTDOWN;

	/* use the buffer lock here for triple duty:
	 *  - prevent I/O (from us) so calling spi_setup() is safe;
	 *  - prevent concurrent SPI_IOC_WR_* from morphing
	 *    data fields while SPI_IOC_RD_* reads them;
	 *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&ad760601->buf_lock);

	switch (cmd) {
	/* read requests */
	case SPI_IOC_RD_MODE:
		retval = __put_user(spi->mode & SPI_MODE_MASK,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_LSB_FIRST:
		retval = __put_user((spi->mode & SPI_LSB_FIRST) ?  1 : 0,
					(__u8 __user *)arg);
		break;
	case SPI_IOC_RD_BITS_PER_WORD:
		retval = __put_user(spi->bits_per_word, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MAX_SPEED_HZ:
		retval = __put_user(spi->max_speed_hz, (__u32 __user *)arg);
		break;

	/* write requests */
	case SPI_IOC_WR_MODE:
		retval = __get_user(tmp, (u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp & ~SPI_MODE_MASK) {
				retval = -EINVAL;
				break;
			}

			tmp |= spi->mode & ~SPI_MODE_MASK;
			spi->mode = (u8)tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "spi mode %02x\n", tmp);
		}
		break;
	case SPI_IOC_WR_LSB_FIRST:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->mode;

			if (tmp)
				spi->mode |= SPI_LSB_FIRST;
			else
				spi->mode &= ~SPI_LSB_FIRST;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->mode = save;
			else
				dev_dbg(&spi->dev, "%csb first\n",
						tmp ? 'l' : 'm');
		}
		break;
	case SPI_IOC_WR_BITS_PER_WORD:
		retval = __get_user(tmp, (__u8 __user *)arg);
		if (retval == 0) {
			u8	save = spi->bits_per_word;

			spi->bits_per_word = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->bits_per_word = save;
			else
				dev_dbg(&spi->dev, "%d bits per word\n", tmp);
		}
		break;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		retval = __get_user(tmp, (__u32 __user *)arg);
		if (retval == 0) {
			u32	save = spi->max_speed_hz;

			spi->max_speed_hz = tmp;
			retval = spi_setup(spi);
			if (retval < 0)
				spi->max_speed_hz = save;
			else
				dev_dbg(&spi->dev, "%d Hz (max)\n", tmp);
		}
		break;

	default:
		/* segmented and/or full-duplex I/O request */
		if (_IOC_NR(cmd) != _IOC_NR(SPI_IOC_MESSAGE(0))
				|| _IOC_DIR(cmd) != _IOC_WRITE) {
			retval = -ENOTTY;
			break;
		}

		tmp = _IOC_SIZE(cmd);
		if ((tmp % sizeof(struct spi_ioc_transfer)) != 0) {
			retval = -EINVAL;
			break;
		}
		n_ioc = tmp / sizeof(struct spi_ioc_transfer);
		if (n_ioc == 0)
			break;

		/* copy into scratch area */
		ioc = kmalloc(tmp, GFP_KERNEL);
		if (!ioc) {
			retval = -ENOMEM;
			break;
		}
		if (__copy_from_user(ioc, (void __user *)arg, tmp)) {
			kfree(ioc);
			retval = -EFAULT;
			break;
		}

		/* translate to spi_message, execute */
		retval = ad760601_message(ad760601, ioc, n_ioc);
		kfree(ioc);
		break;
	}

	mutex_unlock(&ad760601->buf_lock);
	spi_dev_put(spi);
	return retval;
}

#ifdef CONFIG_COMPAT
static long
ad760601_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return ad760601_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define ad760601_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

static int ad760601_open(struct inode *inode, struct file *filp)
{
	struct ad760601_data	*ad760601;
	int			status = -ENXIO;

	mutex_lock(&device_list_lock);

	list_for_each_entry(ad760601, &device_list, device_entry) {
		if (ad760601->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		if (!ad760601->buffer) {
			ad760601->buffer = kmalloc(bufsiz, GFP_KERNEL);
			if (!ad760601->buffer) {
				dev_dbg(&ad760601->spi->dev, "open/ENOMEM\n");
				status = -ENOMEM;
			}
		}
		if (status == 0) {
			ad760601->users++;
			filp->private_data = ad760601;
			nonseekable_open(inode, filp);
		}
	} else
		pr_debug("spidev: nothing for minor %d\n", iminor(inode));

	mutex_unlock(&device_list_lock);
	return status;
}

static int ad760601_release(struct inode *inode, struct file *filp)
{
	struct ad760601_data	*ad760601;
	int status = 0;

	mutex_lock(&device_list_lock);
	ad760601 = filp->private_data;
	filp->private_data = NULL;

	/* last close? */
	ad760601->users--;
	if (!ad760601->users) {
		int		dofree;

		kfree(ad760601->buffer);
		ad760601->buffer = NULL;

		/* ... after we unbound from the underlying device? */
		spin_lock_irq(&ad760601->spi_lock);
		dofree = (ad760601->spi == NULL);
		spin_unlock_irq(&ad760601->spi_lock);

		if (dofree)
			kfree(ad760601);
	}
	mutex_unlock(&device_list_lock);

	return status;
}

static const struct file_operations ad760601_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write = ad760601_write,
	.read = ad760601_read,
	.unlocked_ioctl = ad760601_ioctl,
	.compat_ioctl = ad760601_compat_ioctl,
	.open = ad760601_open,
	.release = ad760601_release,
	.llseek =	no_llseek,
};

/*-------------------------------------------------------------------------*/

/* The main reason to have this class is to make mdev/udev create the
 * /dev/spidevB.C character device nodes exposing our userspace API.
 * It also simplifies memory management.
 */

static struct class *ad760601_class;

/*-------------------------------------------------------------------------*/

static int __devinit ad760601_probe(struct spi_device *spi)
{
	struct ad760601_data *ad760601;
	int status;
	unsigned long minor;

	/* Allocate driver data */
	ad760601 = kzalloc(sizeof(*ad760601), GFP_KERNEL);
	if (!ad760601)
		return -ENOMEM;

	/* reset */
	ad760601_reset();

	/* Initialize the driver data */
	ad760601->spi = spi;
	spin_lock_init(&ad760601->spi_lock);
	mutex_init(&ad760601->buf_lock);

	INIT_LIST_HEAD(&ad760601->device_entry);

	/* If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		ad760601->devt = MKDEV(SPIDEV_MAJOR, minor);

		dev = device_create(ad760601_class, &spi->dev, ad760601->devt,
				    ad760601, "ad760601",
				    spi->master->bus_num, spi->chip_select);

		status = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	} else {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&ad760601->device_entry, &device_list);
	}
	mutex_unlock(&device_list_lock);

	if (status == 0)
		spi_set_drvdata(spi, ad760601);
	else
		kfree(ad760601);

	return status;
}

static int __devexit ad760601_remove(struct spi_device *spi)
{
	struct ad760601_data	*ad760601 = spi_get_drvdata(spi);

	/* make sure ops on existing fds can abort cleanly */
	spin_lock_irq(&ad760601->spi_lock);
	ad760601->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&ad760601->spi_lock);

	/* prevent new opens */
	mutex_lock(&device_list_lock);
	list_del(&ad760601->device_entry);
	device_destroy(ad760601_class, ad760601->devt);
	clear_bit(MINOR(ad760601->devt), minors);
	if (ad760601->users == 0)
		kfree(ad760601);
	mutex_unlock(&device_list_lock);

	return 0;
}

static struct spi_driver ad760601_spi_driver = {
	.driver = {
		.name =	DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe =	ad760601_probe,
	.remove = __devexit_p(ad760601_remove),

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};

/*-------------------------------------------------------------------------*/

static int __init ad760601_init(void)
{
	int status;

	/* Claim our 256 reserved device numbers.  Then register a class
	 * that will key udev/mdev to add/remove /dev nodes.  Last, register
	 * the driver which manages those device numbers.
	 */
	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(SPIDEV_MAJOR, "ad760601_test", &ad760601_fops);
	if (status < 0)
		return status;

	ad760601_class = class_create(THIS_MODULE, "ad760601");
	if (IS_ERR(ad760601_class)) {
		unregister_chrdev(SPIDEV_MAJOR, ad760601_spi_driver.driver.name);
		return PTR_ERR(ad760601_class);
	}

	status = spi_register_driver(&ad760601_spi_driver);
	if (status < 0) {
		class_destroy(ad760601_class);
		unregister_chrdev(SPIDEV_MAJOR, ad760601_spi_driver.driver.name);
	}
	return status;
}
module_init(ad760601_init);

static void __exit ad760601_exit(void)
{
	spi_unregister_driver(&ad760601_spi_driver);
	class_destroy(ad760601_class);
	unregister_chrdev(SPIDEV_MAJOR, ad760601_spi_driver.driver.name);
}
module_exit(ad760601_exit);

MODULE_AUTHOR("topeet: cym");
MODULE_LICENSE("GPL");
//MODULE_ALIAS("spi:spidev");
