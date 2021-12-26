/****************************************************
 * 
 * This file was taken from the following project
 * 
 * https://github.com/hackpascal/ch341prog
 * 
 * All credit for it goes to its author, Weijie Gao
 * 
 ****************************************************/


#include <stdio.h>
#include <string.h>

#include "libusb.h"

#include "ch341.h"

const unsigned char BitSwapTable[256] =
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

static struct libusb_device_handle *CH341DeviceHanlde;

bool CH341DeviceInit(void)
{
	int ret;
	unsigned char desc[0x12];

	if (CH341DeviceHanlde)
		return true;

	if ((ret = libusb_init(NULL)))
	{
		fprintf(stderr, "Error: libusb_init failed: %d (%s)\n", ret, libusb_error_name(ret));
		return false;
	}

	if (!(CH341DeviceHanlde = libusb_open_device_with_vid_pid(NULL, CH341_USB_VID, CH341_USB_PID)))
	{
		fprintf(stderr, "Error: CH341 device (%04x/%04x) not found\n", CH341_USB_VID, CH341_USB_PID);
		return false;
	}

#if !defined(_MSC_VER) && !defined(MSYS) && !defined(CYGWIN) && !defined(WIN32) && !defined(MINGW) && !defined(MINGW32)
	if (libusb_kernel_driver_active(CH341DeviceHanlde, 0))
	{
		if ((ret = libusb_detach_kernel_driver(CH341DeviceHanlde, 0)))
		{
			fprintf(stderr, "Error: libusb_detach_kernel_driver failed: %d (%s)\n", ret, libusb_error_name(ret));
			goto cleanup;
		}
	}
#endif

	if ((ret = libusb_claim_interface(CH341DeviceHanlde, 0)))
	{
		printf("%s:%d\n", __FILE__, __LINE__);

		fprintf(stderr, "Error: libusb_claim_interface failed: %d (%s)\n", ret, libusb_error_name(ret));
		goto cleanup;
	}

	if (!(ret = libusb_get_descriptor(CH341DeviceHanlde, LIBUSB_DT_DEVICE, 0x00, desc, 0x12)))
	{
		printf("%s:%d\n", __FILE__, __LINE__);

		fprintf(stderr, "Warning: libusb_get_descriptor failed: %d (%s)\n", ret, libusb_error_name(ret));
	}

	printf("CH341 %d.%02d found.\n\n", desc[12], desc[13]);

	return true;

cleanup:
	printf("%s:%d\n", __FILE__, __LINE__);

	libusb_close(CH341DeviceHanlde);
	CH341DeviceHanlde = NULL;
	return false;
}

void CH341DeviceRelease(void)
{
	if (!CH341DeviceHanlde)
		return;

	libusb_release_interface(CH341DeviceHanlde, 0);
	libusb_close(CH341DeviceHanlde);
	libusb_exit(NULL);

	CH341DeviceHanlde = NULL;
}

static int CH341USBTransferPart(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int ret, bytestransferred;

	if (!CH341DeviceHanlde)
		return 0;

	if ((ret = libusb_bulk_transfer(CH341DeviceHanlde, CH341_USB_BULK_ENDPOINT | dir, buff, size, &bytestransferred, CH341_USB_TIMEOUT)))
	{
		fprintf(stderr, "Error: libusb_bulk_transfer for IN_EP failed: %d (%s)\n", ret, libusb_error_name(ret));
		return -1;
	}

	return bytestransferred;
}

static bool CH341USBTransfer(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int pos, bytestransferred;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341USBTransferPart(dir, buff + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

#define CH341USBRead(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_IN, buff, size)
#define CH341USBWrite(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_OUT, buff, size)



bool CH341ChipSelect(unsigned int cs, bool enable)
{
	unsigned char pkt[4];

	static const int csio[4] = {0x36, 0x35, 0x33, 0x27};

	if (cs > 3)
	{
		fprintf(stderr, "Error: invalid CS pin %d, 0~3 are available\n", cs);
		return false;
	}

	pkt[0] = CH341_CMD_UIO_STREAM;
	if (enable)
		pkt[1] = CH341_CMD_UIO_STM_OUT | csio[cs];
	else
		pkt[1] = CH341_CMD_UIO_STM_OUT | 0x37;
	pkt[2] = CH341_CMD_UIO_STM_DIR | 0x3F;
	pkt[3] = CH341_CMD_UIO_STM_END;

	return CH341USBWrite(pkt, 4);
}

static int CH341TransferSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	unsigned char pkt[CH341_PACKET_LENGTH];
	unsigned int i;

	if (!size)
		return 0;

	if (size > CH341_PACKET_LENGTH - 1)
		size = CH341_PACKET_LENGTH - 1;

	pkt[0] = CH341_CMD_SPI_STREAM;

	for (i = 0; i < size; i++)
		pkt[i + 1] = BitSwapTable[in[i]];

	if (!CH341USBWrite(pkt, size + 1))
	{
		fprintf(stderr, "Error: failed to transfer data to CH341\n");
		return -1;
	}

	if (!CH341USBRead(pkt, size))
	{
		fprintf(stderr, "Error: failed to transfer data from CH341\n");
		return -1;
	}

	for (i = 0; i < size; i++)
		out[i] = BitSwapTable[pkt[i]];

	return size;
}

bool CH341StreamSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341ReadSPI(unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH];

	if (!size)
		return true;

	memset(pkt, 0, sizeof (pkt));

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(pkt, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341WriteSPI(const unsigned char *in, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH];

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, pkt, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}