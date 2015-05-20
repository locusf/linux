/*
 * FB driver for the HX8357B LCD Controller
 *
 * Copyright (C) 2015 Aleksi Suomalainen
 *
 * Based on driver code found here: https://github.com/wiko-sources/cink-slim/blob/master/mediatek/custom/common/kernel/lcm/hx8357B/hx8357b.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "fbtft.h"
#include "fb_hx8357b.h"

#define DRVNAME		"fb_hx8357b"
#define WIDTH		320
#define HEIGHT		480

/***************** my sekret register writer @ 8MHz */
static int slow_write_spi(struct fbtft_par *par, void *buf, size_t len)
{
  struct spi_transfer t = {
    .tx_buf = buf,
    .len = len,
    .speed_hz = 8000000,
  };
  struct spi_message m;

  fbtft_par_dbg_hex(DEBUG_WRITE, par, par->info->device, u8, buf, len,
                    "%s(len=%d): ", __func__, len);

  if (!par->spi) {
    dev_err(par->info->device,
            "%s: par->spi is unexpectedly NULL\n", __func__);
    return -1;
  }

  spi_message_init(&m);
  if (par->txbuf.dma && buf == par->txbuf.buf) {
    t.tx_dma = par->txbuf.dma;
    m.is_dma_mapped = 1;
  }
  spi_message_add_tail(&t, &m);
  return spi_sync(par->spi, &m);
}

/***************** my sekret register writer @ 8MHz */

static int init_display(struct fbtft_par *par)
{
        /* slow down spi-speed for writing registers */

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	par->fbtftops.reset(par);
	write_reg(par, HX8357B_SWRESET);
	/* Reset things like Gamma */
	write_reg(par, HX8357B_SETPOWER, 
			0x44,
			0x41,
			0x06);
	write_reg(par, HX8357B_SETVCOM,
			0x40,
			0x10);
	write_reg(par, HX8357B_SETPWRNORMAL,
			0x05,
			0x12);
	write_reg(par, HX8357B_SET_PANEL_DRIVING,
			0x14,
			0x3b,
			0x00,
			0x02,
			0x11);
	write_reg(par, HX8357B_SETDISPLAYFRAME,
			0x0c);
	write_reg(par, HX8357B_SETPANELRELATED,
			0x01);
	write_reg(par, HX8357B_SETGAMMA,
			0x00,
			0x15,
			0x00,
			0x22,
			0x00,
			0x08,
			0x77,
			0x26,
			0x77,
			0x22,
			0x04,
			0x00);
	write_reg(par, HX8357B_SETADDRMODE,
			0xc0);
	write_reg(par, HX8357B_SETPIXELFORMAT,
			0x05);
	write_reg(par, HX8357_CASET,
			0x00,
			0x00,
			0x01,
			0x3f);
	write_reg(par, HX8357_PASET,
			0x00,
			0x00,
			0x01,
			0xdf);
	write_reg(par, HX8357B_SETDISPMODE,
			0x00);
	write_reg(par, HX8357_SLPOUT);
	mdelay(300);
	write_reg(par, HX8357_DISPON);	
	write_reg(par, HX8357_RAMWR);
	write_reg(par, HX8357B_WRITEDISPBRIGHT,
			0x8F);
	/* restore user spi-speed */
        udelay(100);

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
        /* slow down spi-speed for writing registers */

	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe, ye);

	/* Column addr set */
	write_reg(par, HX8357_CASET,
		xs >> 8, xs & 0xff,  /* XSTART */
		xe >> 8, xe & 0xff); /* XEND */

	/* Row addr set */
	write_reg(par, HX8357_PASET,
		ys >> 8, ys & 0xff,  /* YSTART */
		ye >> 8, ye & 0xff); /* YEND */

	/* write to RAM */
	write_reg(par, HX8357_RAMWR);

	/* restore user spi-speed */
        //udelay(100);
}
/*
#define HX8357D_MADCTL_MY  0x80
#define HX8357D_MADCTL_MX  0x40
#define HX8357D_MADCTL_MV  0x20
#define HX8357D_MADCTL_ML  0x10
#define HX8357D_MADCTL_RGB 0x00
#define HX8357D_MADCTL_BGR 0x08
#define HX8357D_MADCTL_MH  0x04
static int set_var(struct fbtft_par *par)
{
	u8 val;

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	switch (par->info->var.rotate) {
	case 270:
		val = HX8357D_MADCTL_MV | HX8357D_MADCTL_MY;
		break;
	case 180:
		val = 0;
		break;
	case 90:
		val = HX8357D_MADCTL_MV | HX8357D_MADCTL_MX;
		break;
	default:
		val = HX8357D_MADCTL_MX | HX8357D_MADCTL_MY;
		break;
	}

	val |= (par->bgr ? HX8357D_MADCTL_RGB : HX8357D_MADCTL_BGR);
*/
	/* Memory Access Control */
/*	write_reg(par, HX8357_MADCTL, val);

	return 0;
}*/

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.gamma_num = 2,
	.gamma_len = 14,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		//.set_var = set_var,
	},
};
FBTFT_REGISTER_DRIVER(DRVNAME, "himax,hx8357b", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:hx8357b");
MODULE_ALIAS("platform:hx8357b");

MODULE_DESCRIPTION("FB driver for the HX8357B LCD Controller");
MODULE_AUTHOR("Aleksi Suomalainen <suomalainen.aleksi@gmail.com>");
MODULE_LICENSE("GPL");
