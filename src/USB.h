#pragma once

#include "initialisation.h"

extern uint32_t usbEvents[100];
extern uint8_t usbEventNo, eventOcc;

// USB Definitions
#define USBx_PCGCCTL    *(__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)
#define USBx_DEVICE     ((USB_OTG_DeviceTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USBx_INEP(i)    ((USB_OTG_INEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USBx_OUTEP(i)   ((USB_OTG_OUTEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
//#define USBx_DFIFO(i)   *(__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))
#define USBx_DFIFO(i)   *(uint32_t*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))


// USB Transfer status definitions
#define STS_GOUT_NAK                           1U
#define STS_DATA_UPDT                          2U
#define STS_XFER_COMP                          3U
#define STS_SETUP_COMP                         4U
#define STS_SETUP_UPDT                         6U

// USB Request Recipient types
#define  USB_REQ_RECIPIENT_DEVICE              0x00U
#define  USB_REQ_RECIPIENT_INTERFACE           0x01U
#define  USB_REQ_RECIPIENT_ENDPOINT            0x02U
#define  USB_REQ_RECIPIENT_MASK                0x03U

#define EP_ADDR_MSK                            0xFU

// USB Request types
#define  USB_REQ_TYPE_STANDARD                          0x00U
#define  USB_REQ_TYPE_CLASS                             0x20U
#define  USB_REQ_TYPE_VENDOR                            0x40U
#define  USB_REQ_TYPE_MASK                              0x60U

#define  USB_REQ_GET_STATUS                             0x00U
#define  USB_REQ_CLEAR_FEATURE                          0x01U
#define  USB_REQ_SET_FEATURE                            0x03U
#define  USB_REQ_SET_ADDRESS                            0x05U
#define  USB_REQ_GET_DESCRIPTOR                         0x06U
#define  USB_REQ_SET_DESCRIPTOR                         0x07U
#define  USB_REQ_GET_CONFIGURATION                      0x08U
#define  USB_REQ_SET_CONFIGURATION                      0x09U
#define  USB_REQ_GET_INTERFACE                          0x0AU
#define  USB_REQ_SET_INTERFACE                          0x0BU
#define  USB_REQ_SYNCH_FRAME                            0x0CU

#define  USBD_IDX_LANGID_STR                            0x00U
#define  USBD_IDX_MFC_STR                               0x01U
#define  USBD_IDX_PRODUCT_STR                           0x02U
#define  USBD_IDX_SERIAL_STR                            0x03U
#define  USBD_IDX_CONFIG_STR                            0x04U
#define  USBD_IDX_INTERFACE_STR                         0x05U

#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                 0x06U
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define  USB_DESC_TYPE_BOS                              0x0FU

#define  USB_LEN_DEV_QUALIFIER_DESC                     0x0AU
#define  USB_LEN_DEV_DESC                               0x12U
#define  USB_LEN_CFG_DESC                               0x09U
#define  USB_LEN_IF_DESC                                0x09U
#define  USB_LEN_EP_DESC                                0x07U
#define  USB_LEN_OTG_DESC                               0x03U
#define  USB_LEN_LANGID_STR_DESC                        0x04U
#define  USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09U

#define USBD_VID     1155
#define USBD_LANGID_STRING     1033
#define USBD_MANUFACTURER_STRING     "STMicroelectronics"
#define USBD_PID_FS     22352
#define USBD_PRODUCT_STRING_FS     "STM32 Custom Human interface"
#define USBD_CONFIGURATION_STRING_FS     "Custom HID Config"
#define USBD_INTERFACE_STRING_FS     "Custom HID Interface"

#define  SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
                               (((uint16_t)(*(((uint8_t *)(addr)) + 1U))) << 8U))
#define LOBYTE(x)  ((uint8_t)(x & 0x00FFU))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00U) >> 8U))

struct usbRequest {
	uint8_t mRequest;
	uint8_t Request;
	uint16_t Value;
	uint16_t Index;
	uint16_t Length;
};



class USB {
public:
	void USBInterruptHandler();
	void InitUSB();
	void USB_ActivateEndpoint(uint32_t epnum, bool is_in, uint8_t eptype);
	void USB_ReadPacket(uint32_t *dest, uint16_t len);
	void USB_WritePacket(uint8_t *src, uint32_t ch_ep_num, uint16_t len);
	void USBD_GetDescriptor(usbRequest req);
	void USBD_StdDevReq (usbRequest req);
	void USB_EP0StartXfer(bool is_in, uint8_t epnum, uint8_t* xfer_buff, uint32_t xfer_len);
	uint32_t USB_ReadInterrupts();

	usbRequest req;
	uint8_t maxPacket = 0x40;
	uint32_t xfer_buff[32];		// in HAL there is a transfer buffer for each in and out endpoint
	uint32_t xfer_count;
	//	FIXME - should there be one of these output buffers for each endpoint?
	uint8_t* outBuff;
	uint32_t outBuffSize;
	uint32_t outCount;

	// USB standard device descriptor
	uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] = {
		0x12,					// bLength
		USB_DESC_TYPE_DEVICE,	// bDescriptorType
		0x01,					// bcdUSB  - 0x01 if LPM enabled
		0x02,
		0x00,					// bDeviceClass
		0x00,					// bDeviceSubClass
		0x00,					// bDeviceProtocol
		64,  				 	// bMaxPacketSize
		LOBYTE(USBD_VID),		// idVendor
		HIBYTE(USBD_VID),		// idVendor
		LOBYTE(USBD_PID_FS),	// idProduct
		HIBYTE(USBD_PID_FS),	// idProduct
		0x00,					// bcdDevice rel. 2.00
		0x02,
		USBD_IDX_MFC_STR,		// Index of manufacturer  string
		USBD_IDX_PRODUCT_STR,	// Index of product string
		USBD_IDX_SERIAL_STR,	// Index of serial number string
		1						// bNumConfigurations
	};
};
