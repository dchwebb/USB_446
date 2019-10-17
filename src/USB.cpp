#include <USB.h>

void USB::USBInterruptHandler() {
	/* startup sequence:
	40000000 USB_OTG_GINTSTS_SRQINT (Session request/new session detected) 30
	800 USB_OTG_GINTSTS_USBSUSP (USB suspend) 11
	80000000 USB_OTG_GINTSTS_WKUINT (Resume/remote wakeup detected) 31
	1000 USB_OTG_GINTSTS_USBRST (USB reset) 12
	2000 USB_OTG_GINTSTS_ENUMDNE (Enumeration done) 13
	10 		RXFLVL (RxFIFO non-empty)		receives Setup in packet
	10
	80000	OEPINT 				USB_OTG_DOEPINT_STUP (device descriptor)
	40000	IEPINT  			USB_OTG_DIEPINT_TXFE  Transmit FIFO empty
	40000	IEPINT  			USB_OTG_DIEPINT_XFRC  Transfer completed interrupt
[	80000					10	USB_OTG_DOEPINT_NAK - HAL only]
	10
	10
	80000						USB_OTG_DOEPINT_XFRC
	10							Address setup happens here
	10
	80000	OEPINT			15	USB_OTG_DOEPINT_STUP (address)
	40000	IEPINT			16	USB_OTG_DIEPINT_XFRC
	10						17	STS_SETUP_UPDT reads 0x1000680 0x120000
	10						18	STS_SETUP_COMP (does nothing)
	80000	OEPINT 			19	USB_OTG_DOEPINT_STUP [asks for device descriptor again but with advice address (rather than 0)]
	40000	IEPINT			20	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			21	USB_OTG_DIEPINT_XFRC
	10						22	STS_DATA_UPDT
	10						23	STS_SETUP_UPDT [reads 0x1000680 0x400000 NB in HAL this is STS_XFER_COMP]
	80000	OEPINT 			24	USB_OTG_DOEPINT_XFRC
	10						25	STS_SETUP_UPDT reads 0x2000680 0xff0000 [request for configuration descriptor]
	10						26	STS_SETUP_COMP
	80000	OEPINT 			27	USB_OTG_DOEPINT_STUP
	40000	IEPINT			28	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			29	USB_OTG_DIEPINT_XFRC
	10						30	STS_DATA_UPDT
	10						31	STS_SETUP_UPDT
	80000	OEPINT 			32	USB_OTG_DOEPINT_XFRC
	10						33	STS_SETUP_UPDT reads 0xF000680 0xff0000 [request for BOS descriptor]
	10						34	STS_SETUP_COMP
	80000	OEPINT 			35	USB_OTG_DOEPINT_STUP
	40000	IEPINT			36	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			37	USB_OTG_DIEPINT_XFRC
	10						38	STS_DATA_UPDT
	10						39	STS_SETUP_UPDT
	80000	OEPINT 			40	USB_OTG_DOEPINT_XFRC
	10						41	STS_SETUP_UPDT reads 0x3030680 0xff0409 [request for string descriptor]
	10						42	STS_SETUP_COMP
	80000	OEPINT 			43	USB_OTG_DOEPINT_STUP
	40000	IEPINT			44	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			45	USB_OTG_DIEPINT_XFRC
	10						46	STS_DATA_UPDT
	10						47	STS_SETUP_UPDT
	80000	OEPINT 			48	USB_OTG_DOEPINT_XFRC
	10						49	STS_SETUP_UPDT reads 0x3000680 0xff0000 [request for string language ID]
	10						50	STS_SETUP_COMP
	80000	OEPINT 			51	USB_OTG_DOEPINT_STUP
	40000	IEPINT			52	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			53	USB_OTG_DIEPINT_XFRC
	10						54	STS_DATA_UPDT
	10						55	STS_SETUP_UPDT
	80000	OEPINT 			56	USB_OTG_DOEPINT_XFRC
	10						57	STS_SETUP_UPDT reads 0x3020680 0xff0409 [request for product string]
	10						58	STS_SETUP_COMP
	80000	OEPINT 			59	USB_OTG_DOEPINT_STUP
	40000	IEPINT			60	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			61	USB_OTG_DIEPINT_XFRC
	10						62	STS_DATA_UPDT
	10						63	STS_SETUP_UPDT
	80000	OEPINT 			64	USB_OTG_DOEPINT_XFRC
	10						65	STS_SETUP_UPDT reads 0x6000680 0xa0000 [request for USB_DESC_TYPE_DEVICE_QUALIFIER] > Stall
	10						66	STS_SETUP_COMP
	80000	OEPINT 			67	USB_OTG_DOEPINT_STUP
	10						68	STS_DATA_UPDT
	10						69	STS_SETUP_UPDT
	80000	OEPINT 			70	USB_OTG_DOEPINT_STUP 0x10900 [Set configuration to 1]
	40000	IEPINT			71	USB_OTG_DIEPINT_XFRC
	10						72	STS_DATA_UPDT
	10						73	STS_SETUP_UPDT
	80000	OEPINT 			74	USB_OTG_DOEPINT_STUP 0xA21 [USB_REQ_RECIPIENT_INTERFACE]
	40000	IEPINT			75	USB_OTG_DIEPINT_XFRC
	10						76	STS_DATA_UPDT
	10						77	STS_SETUP_UPDT
	80000	OEPINT 			78	USB_OTG_DOEPINT_STUP 0x22000681 0x8a0000
	40000	IEPINT			79	USB_OTG_DIEPINT_TXFE Send first part of CUSTOM_HID_ReportDesc_FS
	40000	IEPINT			80	USB_OTG_DIEPINT_XFRC
	40000	IEPINT			81	USB_OTG_DIEPINT_TXFE Send second part of CUSTOM_HID_ReportDesc_FS
	40000	IEPINT			82	USB_OTG_DIEPINT_XFRC
	10						83	STS_DATA_UPDT
	10						84	STS_SETUP_UPDT
	80000	OEPINT 			85	USB_OTG_DOEPINT_XFRC
	10						86	STS_SETUP_UPDT reads 0x6000680 0xa0000 [request for USB_DESC_TYPE_DEVICE_QUALIFIER] > Stall
	10						87	STS_SETUP_COMP
	80000	OEPINT 			88	USB_OTG_DOEPINT_STUP
	10						89	STS_SETUP_UPDT reads 0x3000680 0x1fe0000
	10						90	STS_SETUP_COMP
	80000	OEPINT 			91	USB_OTG_DOEPINT_STUP req:80,6,300,0,1FE String descriptor: USBD_IDX_LANGID_STR
	40000	IEPINT			92	USB_OTG_DIEPINT_TXFE
	40000	IEPINT			93	USB_OTG_DIEPINT_XFRC
	10						94	STS_DATA_UPDT
	10						95	STS_SETUP_UPDT
	80000	OEPINT 			96	USB_OTG_DOEPINT_STUP
	10						97	STS_SETUP_UPDT reads 0x3010680 0x1fe0409
	10						98	STS_SETUP_COMP
	80000	OEPINT 			99	USB_OTG_DOEPINT_STUP req:80,6,301,409,1FE String descriptor: USBD_IDX_LANGID_STR
	..
	80000	OEPINT 			115	USB_OTG_DOEPINT_STUP req:80,6,3EE,409,1FE String descriptor: Custom user string?
	 */

	int epnum, ep_intr, epint;

	//int interruptCode = USB_ReadInterrupts();

	if (usbEventNo >= 116) {
		int susp = 1;
	}

	// Handle spurious interrupt
	if ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) == 0)
		return;

	// Handle Mode mismatch interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_MMIS)) {
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_MMIS;
	}

	/////////////////////////////////////////////// 80000 OEPINT
	// OUT endpoint interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_OEPINT)) {

		if (usbEventNo > 115) {
			int susp = 1;
		}
		// Read the output endpoint interrupt register to ascertain which endpoint(s) fired an interrupt
		ep_intr = ((USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFF0000U) >> 16; // FIXME mask unnecessary with shift right?


		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0) {
			if ((ep_intr & 1) != 0) {
				epint = USBx_OUTEP(epnum)->DOEPINT & USBx_DEVICE->DOEPMSK;

				if ((epint & USB_OTG_DOEPINT_XFRC) == USB_OTG_DOEPINT_XFRC) {		// Transfer completed
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_XFRC;				// Clear interrupt
					//(void)PCD_EP_OutXfrComplete_int(hpcd, epnum);					// Appears to lead to placeholder function, overridable in user program
					//if (pdev->ep0_state == USBD_EP0_STATUS_OUT)
			        // STATUS PHASE completed, update ep0_state to idle
			        ep0_state = USBD_EP0_IDLE;
					//USBD_LL_StallEP(pdev, 0U);
				    USBx_OUTEP(epnum)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
				}

				if ((epint & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP) {		// SETUP phase done
					/* Class B setup phase done for previous decoded setup */

					//(void)PCD_EP_OutSetupPacket_int(hpcd, epnum);		// This is where the in ep is enabled

					//USBD_ParseSetupRequest(&pdev->request, psetup);
					//NB psetup refers to hcpd.Setup which is an array[12] named 'setup buffer' filled by USB_OTG_GINTSTS_RXFLVL



					// Parse Setup Request containing data in xfer_buff filled by RXFLVL interrupt
					uint8_t *pdata = (uint8_t*)xfer_buff;
					req.mRequest     = *(uint8_t *)  (pdata);
					req.Request      = *(uint8_t *)  (pdata +  1);
					req.Value        = SWAPBYTE      (pdata +  2);
					req.Index        = SWAPBYTE      (pdata +  4);
					req.Length       = SWAPBYTE      (pdata +  6);

					//pdev->ep0_data_len = pdev->request.wLength;
					ep0_state = USBD_EP0_SETUP;
					switch (req.mRequest & 0x1F) {		// originally USBD_LL_SetupStage
					case USB_REQ_RECIPIENT_DEVICE:
						//initially USB_REQ_GET_DESCRIPTOR >> USB_DESC_TYPE_DEVICE (bmrequest is 0x6)
						USBD_StdDevReq(req);
						break;

					case USB_REQ_RECIPIENT_INTERFACE:
						if (req.mRequest == 0x21) {
							USB_EP0StartXfer(DIR_IN, 0, 0);		// sends blank request back
						} else if (req.mRequest == 0x81) {
							//USBD_CUSTOM_HID_Setup > USB_REQ_GET_DESCRIPTOR
							// FIXME - move to GetDescriptor?
							if (req.Value >> 8 == 0x22)		// 0x22 = CUSTOM_HID_REPORT_DESC
							{
								outBuffSize = std::min((uint16_t)0x4A, req.Length);		// 0x4A = USBD_CUSTOM_HID_REPORT_DESC_SIZE
								outBuff = CUSTOM_HID_ReportDesc_FS;
								ep0_state = USBD_EP0_DATA_IN;
								USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
							}
							else
							{
								if (req.Value >> 8 == 0x21)		// 0x21 = CUSTOM_HID_DESCRIPTOR_TYPE
								{
									/*outBuff = USBD_CUSTOM_HID_Desc;
									outBuffSize = std::min(USB_CUSTOM_HID_DESC_SIZ , req.Length);*/
								}
							}
						}
						//USBD_StdItfReq(pdev, req);
						break;

					case USB_REQ_RECIPIENT_ENDPOINT:
						//USBD_StdEPReq(pdev, req);
						break;

					default:
						//USBD_LL_StallEP(pdev, (req.mRequest & 0x80U));
						break;
					}

					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_STUP;				// Clear interrupt
				}

				if ((epint & USB_OTG_DOEPINT_OTEPDIS) == USB_OTG_DOEPINT_OTEPDIS) {	// OUT token received when endpoint disabled
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPDIS;			// Clear interrupt
				}
				if ((epint & USB_OTG_DOEPINT_OTEPSPR) == USB_OTG_DOEPINT_OTEPSPR) {	// Status Phase Received interrupt
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPSPR;			// Clear interrupt
				}
				if ((epint & USB_OTG_DOEPINT_NAK) == USB_OTG_DOEPINT_NAK) {			// OUT NAK interrupt
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_NAK;				// Clear interrupt
				}
			}
			epnum++;
			ep_intr >>= 1U;
		}

	}

	/////////////////////////////////////////////// 40000 IEPINT
	// IN endpoint interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_IEPINT))
	{


		// Read in the device interrupt bits [initially 1]
		ep_intr = (USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFFU;

		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0U) {
			if ((ep_intr & 1) != 0) { // In ITR [initially true]
				// epint = USB_ReadDevInEPInterrupt(hpcd->Instance, (uint8_t)epnum);

				// [initially 0x80]
				epint = USBx_INEP((uint32_t)epnum)->DIEPINT & (USBx_DEVICE->DIEPMSK | (((USBx_DEVICE->DIEPEMPMSK >> (epnum & EP_ADDR_MSK)) & 0x1U) << 7));

				if ((epint & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC) {
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_XFRC;

					if (epnum == 0) {

						//HAL_PCD_DataInStageCallback(hpcd, (uint8_t)epnum);

						if (ep0_state == USBD_EP0_DATA_IN) {
							if (xfer_rem > 0) {
								outBuffSize = xfer_rem;
								xfer_rem = 0;
								USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
							} else {

								USB_EPSetStall(epnum);

								ep0_state = USBD_EP0_STATUS_OUT;

								//HAL_PCD_EP_Receive
								xfer_rem = 0;
								xfer_buff[0] = 0;
								USB_EP0StartXfer(DIR_OUT, 0, ep0_maxPacket);
							}
						}
						else if ((ep0_state == USBD_EP0_STATUS_IN) || (ep0_state == USBD_EP0_IDLE)) {		// second time around
							USB_EPSetStall(epnum);
						}
					} else {
						int susp = 1;
						hid_state = CUSTOM_HID_IDLE;
					}
				}


				if ((epint & USB_OTG_DIEPINT_TOC) == USB_OTG_DIEPINT_TOC) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_TOC;
				}
				if ((epint & USB_OTG_DIEPINT_ITTXFE) == USB_OTG_DIEPINT_ITTXFE) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_ITTXFE;
				}
				if ((epint & USB_OTG_DIEPINT_INEPNE) == USB_OTG_DIEPINT_INEPNE) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_INEPNE;
				}
				if ((epint & USB_OTG_DIEPINT_EPDISD) == USB_OTG_DIEPINT_EPDISD) {
					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_EPDISD;
				}
				if ((epint & USB_OTG_DIEPINT_TXFE) == USB_OTG_DIEPINT_TXFE) {
					//(void)PCD_WriteEmptyTxFifo(hpcd, epnum);


					uint32_t maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);
					if (outBuffSize > maxPacket) {
						xfer_rem = outBuffSize - maxPacket;
						outBuffSize = maxPacket;
					}

					USB_WritePacket(outBuff, epnum, (uint16_t)outBuffSize);

					outBuff += outBuffSize;		// Move pointer forwards
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

				}
			}
			epnum++;
			ep_intr >>= 1U;
		}

	}

	/////////////////////////////////////////////// 80000000 USB_OTG_GINTSTS_WKUINT
	// Handle Resume Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_WKUINT))
	{
		// Clear the Remote Wake-up Signaling
		USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_WKUINT;
	}

	/////////////////////////////////////////////// 800 USBSUSP
	// Handle Suspend Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_USBSUSP))
	{

		if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_SUSPSTS) == USB_OTG_DSTS_SUSPSTS)
		{
			//USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);		// stores status of suspend pdev->
			dev_state  = USBD_STATE_SUSPENDED;
			USBx_PCGCCTL |= USB_OTG_PCGCCTL_STOPCLK;
		}
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBSUSP;
	}

	/////////////////////////////////////////////// 1000 USB_OTG_GINTSTS_USBRST
	// Handle Reset Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_USBRST))
	{
		USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		// USB_FlushTxFifo
		USB_OTG_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (0x10 << 6));
		while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);

		for (int i = 0; i < 6; i++) {				// hpcd->Init.dev_endpoints
			USBx_INEP(i)->DIEPINT = 0xFB7FU;		// see p1177 for explanation: based on datasheet should be more like 0b10100100111011
			USBx_INEP(i)->DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
			USBx_OUTEP(i)->DOEPINT = 0xFB7FU;
			USBx_OUTEP(i)->DOEPCTL &= ~USB_OTG_DOEPCTL_STALL;
		}
		USBx_DEVICE->DAINTMSK |= 0x10001U;

		USBx_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_STUPM |
				USB_OTG_DOEPMSK_XFRCM |
				USB_OTG_DOEPMSK_EPDM |
				USB_OTG_DOEPMSK_OTEPSPRM |
				USB_OTG_DOEPMSK_NAKM;

		USBx_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_TOM |
				USB_OTG_DIEPMSK_XFRCM |
				USB_OTG_DIEPMSK_EPDM;

		// Set Default Address to 0
		USBx_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;

		// setup EP0 to receive SETUP packets
		if ((USBx_OUTEP(0U)->DOEPCTL & USB_OTG_DOEPCTL_EPENA) != USB_OTG_DOEPCTL_EPENA)	{
			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
			USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));		// Set PKTCNT to 1
			USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);									// Set XFRSIZ to 24
			USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;					// Set STUPCNT to 3
		}

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBRST;
	}

	/////////////////////////////////////////////// 2000 USB_OTG_GINTSTS_ENUMDNE
	// Handle Enumeration done Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_ENUMDNE))
	{
		// Set the MPS of the IN EP based on the enumeration speed
		USBx_INEP(0U)->DIEPCTL &= ~USB_OTG_DIEPCTL_MPSIZ;
#define DSTS_ENUMSPD_LS_PHY_6MHZ               (2U << 1)
		if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_ENUMSPD) == DSTS_ENUMSPD_LS_PHY_6MHZ) {
			USBx_INEP(0U)->DIEPCTL |= 3U;
		}
		USBx_DEVICE->DCTL |= USB_OTG_DCTL_CGINAK;		//  Clear global IN NAK

		// Assuming Full Speed USB and clock > 32MHz Set USB Turnaround time
		int UsbTrd = 0x6U;

		USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_TRDT;
		USB_OTG_FS->GUSBCFG |= (uint32_t)((UsbTrd << 10) & USB_OTG_GUSBCFG_TRDT);

		// Set Speed. pdev->dev_speed = 1

		USB_ActivateEndpoint(0, DIR_OUT, 0);		// Open EP0 OUT
		USB_ActivateEndpoint(0, DIR_IN, 0);		// Open EP0 IN

		ep0_state = USBD_EP0_IDLE;

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_ENUMDNE;
	}

	/////////////////////////////////////////////// 10 RXFLVL
	// Handle RxQLevel Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_RXFLVL))
	{

		USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;

		uint32_t temp = USB_OTG_FS->GRXSTSP;		//OTG receive status debug read/OTG status read and	pop registers (OTG_GRXSTSR/OTG_GRXSTSP) not shown in SFR

		// Get the endpoint number
		epnum = temp & USB_OTG_GRXSTSP_EPNUM;

		if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_DATA_UPDT) {
			if ((temp & USB_OTG_GRXSTSP_BCNT) != 0)
			{
				USB_ReadPacket(xfer_buff, (temp & USB_OTG_GRXSTSP_BCNT) >> 4);

				//xfer_buff += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
				//xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
			}
		}
		else if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) ==  STS_SETUP_UPDT) {
			USB_ReadPacket(xfer_buff, 8U);
			//xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
		}

		USB_OTG_FS->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
	}

	/////////////////////////////////////////////// 8 SOF
	// Handle SOF Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_SOF))
	{
		// Doesn't seem to do anything
		//HAL_PCD_SOFCallback(hpcd);

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SOF;
	}

	/* Handle Incomplete ISO IN Interrupt */
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_IISOIXFR))
	{
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_IISOIXFR;
	}

	/* Handle Incomplete ISO OUT Interrupt */
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_PXFR_INCOMPISOOUT))
	{
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_PXFR_INCOMPISOOUT;
	}

	/////////////////////////////////////////////// 40000000 SRQINT
	// Handle Connection event Interrupt
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_SRQINT))
	{
		//HAL_PCD_ConnectCallback(hpcd);		// this doesn't seem to do anything

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SRQINT;
	}

	/* Handle Disconnection event Interrupt */
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_OTGINT))
	{
		uint32_t temp = USB_OTG_FS->GOTGINT;

		if ((temp & USB_OTG_GOTGINT_SEDET) == USB_OTG_GOTGINT_SEDET)
		{
			//HAL_PCD_DisconnectCallback(hpcd);
			//pdev->pClass->DeInit(pdev, (uint8_t)pdev->dev_config);
		}
		USB_OTG_FS->GOTGINT |= temp;
	}

	/* Link Power Management
	// Handle LPM Interrupt
#if defined(STM32F446xx) || defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F412Zx) || defined(STM32F412Vx) || defined(STM32F412Rx) || defined(STM32F412Cx) || defined(STM32F413xx) || defined(STM32F423xx)
	if (USB_ReadInterrupts(USB_OTG_GINTSTS_LPMINT))
	{
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_LPMINT;

		if (hpcd->LPM_State == LPM_L0)
		{
			hpcd->LPM_State = LPM_L1;
			hpcd->BESL = (hpcd->Instance->GLPMCFG & USB_OTG_GLPMCFG_BESL) >> 2U;

			HAL_PCDEx_LPM_Callback(hpcd, PCD_LPM_L1_ACTIVE);
		}
		else
		{
			HAL_PCD_SuspendCallback(hpcd);
		}
	}
#endif
	 */

}



void USB::InitUSB()
{
	// *********************  HAL_PCD_MspInit() in /Src/usbd_conf.c

	// USB_OTG_FS GPIO Configuration: PA8: USB_OTG_FS_SOF; PA9: USB_OTG_FS_VBUS; PA10: USB_OTG_FS_ID; PA11: USB_OTG_FS_DM; PA12: USB_OTG_FS_DP
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	// PA8 (SOF), PA10 (ID), PA11 (DM), PA12 (DP) (NB PA9 - VBUS uses default values)
	GPIOA->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;					// 10: Alternate function mode
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12;		// 11: High speed
	GPIOA->AFR[1] |= (10 << 12) | (10 << 16);															// Alternate Function 10 is OTG_FS

/*
	GPIOA->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;					// 10: Alternate function mode
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12;		// 11: High speed
	GPIOA->AFR[1] |= (10 << 0) | (10 << 8) | (10 << 12) | (10 << 16);															// Alternate Function 10 is OTG_FS
*/


	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;				// USB OTG FS clock enable
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;				// Enable system configuration clock: used to manage external interrupt line connection to GPIOs

	NVIC_SetPriority(OTG_FS_IRQn, 0);
	NVIC_EnableIRQ(OTG_FS_IRQn);

/*
	// *********************  HAL_PCD_Init() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
	USB_OTG_FS->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;		// Disable global interrupts

	// *********************  USB_CoreInit in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;		// Select FS interface (embedded Phy)

	// *********************  USB_CoreReset() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	// Reset the USB Core (needed after USB clock settings change)

	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U);
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;		// Core Soft Reset
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);
*/


	// *********************  USB_CoreInit() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;			// Activate the transceiver in transmission/reception. When reset, the transceiver is kept in power-down. 0 = USB FS transceiver disabled; 1 = USB FS transceiver enabled

	// *********************  USB_SetCurrentMode() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;		// Force USB device mode
	//HAL_Delay(50U);

	// *********************  USB_DevInit() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c

	// Not really sure what this is doing?
	// OTG device IN endpoint transmit FIFO size register	(OTG_DIEPTXFx) (x = 1..5[FS] /8[HS], where x is the	FIFO number)
	// Bits 31:16 INEPTXFD[15:0]: IN endpoint Tx FIFO depth
	// Bits 15:0 INEPTXSA[15:0]: IN endpoint FIFOx transmit RAM start address
	for (uint8_t i = 0U; i < 15U; i++) {
		USB_OTG_FS->DIEPTXF[i] = 0U;
	}


	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN; 			// Enable HW VBUS sensing

	/*
	USBx_PCGCCTL &= ~USB_OTG_PCGCCTL_STOPCLK;
	USBx_DEVICE->DCFG &= ~USB_OTG_DCFG_PFIVL;			// default 00: 80% of the frame interval: Indicates time within frame at which application must be notified using end of periodic frame interrupt. This can be used to determine if all the isochronous traffic for that frame is complete.
	*/

	// *********************  USB_SetDevSpeed() in  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USBx_DEVICE->DCFG |= USB_OTG_DCFG_DSPD;				// 11: Full speed using internal FS PHY

	// *********************  USB_FlushTxFifo() and USB_FlushRxFifo() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFNUM_4;	// Select buffers to flush. 10000: Flush all the transmit FIFOs in device or host mode
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_TXFFLSH;		// Flush the TX buffers
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH);

	USB_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH);

	// ********************* continues USB_DevInit() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	/* Clear all pending Device Interrupts */
	/*
	USBx_DEVICE->DIEPMSK = 0U;
	USBx_DEVICE->DOEPMSK = 0U;
	USBx_DEVICE->DAINTMSK = 0U;

	// Note these following commands do not visibly change anything - some of these appear to affect other registers (see p 1177)
	for (int i = 0U; i < 6; i++) {						// 6 = cfg.dev_endpoints
		USBx_INEP(i)->DIEPCTL = 0U;						// shown in SFRs as FS_DIEPCTL0, DIEPCTL1, DIEPCTL2, DIEPCTL3 (??DIEPCTL4/5 not shown in SFR)
		USBx_INEP(i)->DIEPTSIZ = 0U;
		USBx_INEP(i)->DIEPINT  = 0xFB7FU;				// OTG device IN endpoint x interrupt register
	}

	for (int i = 0U; i < 6; i++) {
		USBx_OUTEP(i)->DOEPCTL = 0U;
		USBx_OUTEP(i)->DOEPTSIZ = 0U;
		USBx_OUTEP(i)->DOEPINT  = 0xFB7FU;
	}
*/
	//USBx_DEVICE->DIEPMSK &= ~(USB_OTG_DIEPMSK_TXFURM);	// FIFO underrun mask

	USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;					// Clear pending interrupts (except SRQINT Session request/new session detected)

	// Enable interrupts
	USB_OTG_FS->GINTMSK = 0U;							// Disable all interrupts
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM | USB_OTG_GINTMSK_USBSUSPM |			// Receive FIFO non-empty mask; USB suspend
			USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM |							// USB reset; Enumeration done
			USB_OTG_GINTMSK_IEPINT | USB_OTG_GINTMSK_OEPINT | USB_OTG_GINTMSK_WUIM |	// IN endpoint; OUT endpoint; Resume/remote wakeup detected
			USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT;								// Session request/new session detected; OTG interrupt

	/*
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;		// Enable the Global interrupt: Receive FIFO non-empty mask

	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |	// USB suspend; USB reset
			USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |					// Enumeration done; IN endpoints interrupt
			USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM |				// OUT endpoints interrupt; Incomplete isochronous IN transfer
			USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM;				// Incomplete isochronous OUT transfer; Resume/remote wakeup detected

	//USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_SOFM;								// Start of frame
	USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT);	// Session request/new session detected; OTG interrupt

*/

	// ********************* USB_DevDisconnect() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	// As long as this bit is set, the host does not see that the device is connected, and the device does not receive signals on the USB.
	//USBx_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;				// Soft disconnect (not needed: already set by RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN)
	//HAL_Delay(3U);

	// ********************* HAL_PCDEx_SetRxFiFo() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
	USB_OTG_FS->GRXFSIZ = 128;		 					// RxFIFO depth

	// ********************* HAL_PCDEx_SetTxFiFo() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
	// Non-periodic transmit FIFO size register (FS_GNPTXFSIZ_Device in SFR)
	USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = ((uint32_t)64 << USB_OTG_TX0FD_Pos) |		// Endpoint 0 TxFIFO depth
			((uint32_t)128 << USB_OTG_TX0FSA_Pos);								// Endpoint 0 transmit RAM start  address

    // Multiply Tx_Size by 2 to get higher performance
    USB_OTG_FS->DIEPTXF[0] = ((uint32_t)128 << USB_OTG_DIEPTXF_INEPTXFD_Pos) |	// IN endpoint TxFIFO depth
    		((uint32_t)192 << USB_OTG_DIEPTXF_INEPTXSA_Pos);  					// IN endpoint FIFO2 transmit RAM start address


    // *********************  USB_DevConnect() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
    USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;

    // *********************  USB_EnableGlobalInt() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
}


void USB::USB_ActivateEndpoint(uint32_t epnum, bool is_in, uint8_t eptype)
{
	uint8_t maxpktsize = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);

	if (is_in) {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (epnum & EP_ADDR_MSK));

		if ((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_USBAEP) == 0U) {
			USBx_INEP(epnum)->DIEPCTL |= (maxpktsize & USB_OTG_DIEPCTL_MPSIZ) |
					((uint32_t)eptype << 18) | (epnum << 22) |
					USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
					USB_OTG_DIEPCTL_USBAEP;
		}
	} else {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (epnum & EP_ADDR_MSK)) << 16);

		if (((USBx_OUTEP(epnum)->DOEPCTL) & USB_OTG_DOEPCTL_USBAEP) == 0U) {
			USBx_OUTEP(epnum)->DOEPCTL |= (maxpktsize & USB_OTG_DOEPCTL_MPSIZ) |
					((uint32_t)eptype << 18) |
					USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
					USB_OTG_DOEPCTL_USBAEP;
		}
	}

}

// USB_ReadPacket : read a packet from the RX FIFO
void USB::USB_ReadPacket(uint32_t *dest, uint16_t len)
{
	uint32_t *pDest = (uint32_t *)dest;
	uint32_t count32b = ((uint32_t)len + 3U) / 4U;

	for (uint32_t i = 0; i < count32b; i++)
	{
		*pDest = USBx_DFIFO(0U);
		pDest++;
	}

	//return ((void *)pDest);
}

void USB::USB_WritePacket(uint8_t *src, uint32_t ch_ep_num, uint16_t len)
{
	uint32_t *pSrc = (uint32_t *)src;
	uint32_t count32b, i;

	count32b = ((uint32_t)len + 3U) / 4U;
	for (i = 0; i < count32b; i++) {
		USBx_DFIFO(ch_ep_num) = *pSrc;
		pSrc++;
	}


}


// Descriptors in usbd_desc.c
void USB::USBD_GetDescriptor(usbRequest req)
{
	uint16_t len;
	uint8_t *pbuf;
	uint32_t deviceserial0, deviceserial1, deviceserial2;

	if (usbEventNo >= 115) {
		int susp = 1;
	}

	switch (req.Value >> 8)
	{
	case USB_DESC_TYPE_DEVICE:
		//pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
		outBuff = USBD_FS_DeviceDesc;
		outBuffSize = sizeof(USBD_FS_DeviceDesc);
		break;

	case USB_DESC_TYPE_CONFIGURATION:

		outBuff = USBD_CUSTOM_HID_CfgFSDesc;
		outBuffSize = sizeof(USBD_CUSTOM_HID_CfgFSDesc);
		//pbuf   = (uint8_t *)pdev->pClass->GetFSConfigDescriptor(&len);
		//pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
		break;

	case USB_DESC_TYPE_BOS:

		outBuff = USBD_FS_BOSDesc;
		outBuffSize = sizeof(USBD_FS_BOSDesc);

		//pbuf = pdev->pDesc->GetBOSDescriptor(pdev->dev_speed, &len);
		break;

	case USB_DESC_TYPE_STRING:
		switch ((uint8_t)(req.Value)) {
		case USBD_IDX_LANGID_STR:
			outBuff = USBD_LangIDDesc;
			outBuffSize = sizeof(USBD_LangIDDesc);
			//pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_MFC_STR:
			outBuffSize = USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			//pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_PRODUCT_STR:
			outBuffSize = USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc);
			outBuff = USBD_StrDesc;
			//pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_SERIAL_STR:
			// STM32 unique device ID (96 bit number starting at UID_BASE)
			deviceserial0 = *(uint32_t *) UID_BASE;
			deviceserial1 = *(uint32_t *) UID_BASE + 4;
			deviceserial2 = *(uint32_t *) UID_BASE + 8;
			deviceserial0 += deviceserial2;

			if (deviceserial0 != 0)
			{
				IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
				IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
			}
			outBuff = USBD_StringSerial;
			outBuffSize = sizeof(USBD_StringSerial);
			//pbuf = pdev->pDesc->GetSerialStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_CONFIG_STR:
			//pbuf = pdev->pDesc->GetConfigurationStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_INTERFACE_STR:
			//pbuf = pdev->pDesc->GetInterfaceStrDescriptor(pdev->dev_speed, &len);
			break;
		default:
#if (USBD_SUPPORT_USER_STRING == 1U)
			//pbuf = pdev->pClass->GetUsrStrDescriptor(pdev, (req->wValue) , &len);
			break;
#else
			USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
			USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
			USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
			USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

			USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;
			return;
#endif
		}
		break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			//USBD_CtlError(pdev , req);
			USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

			USBx_OUTEP(0U)->DOEPTSIZ = 0U;
			USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
			USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
			USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

			USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_STALL;

			return;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			//USBD_CtlError(pdev , req);
			return;
		default:
			//USBD_CtlError(pdev , req);
			return;
	}

	if ((outBuffSize != 0U) && (req.Length != 0U)) {
		ep0_state = USBD_EP0_DATA_IN;
		outBuffSize = std::min(outBuffSize, (uint32_t)req.Length);
		USB_EP0StartXfer(DIR_IN, 0, outBuffSize);
	}

	if (req.Length == 0U) {
		USB_EP0StartXfer(DIR_IN, 0, 0);
	}
}

uint32_t USB::USBD_GetString(uint8_t *desc, uint8_t *unicode)
{
	uint32_t idx = 2;

	if (desc != NULL)
	{
		while (*desc != '\0') {
			unicode[idx++] = *desc++;
			unicode[idx++] =  0U;
		}
		unicode[0] = idx;
		unicode[1] = USB_DESC_TYPE_STRING;
	}
	return idx;
}

void USB::IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len) {

	for (uint8_t idx = 0; idx < len; idx++) {
		if (((value >> 28)) < 0xA) {
			pbuf[2 * idx] = (value >> 28) + '0';
		} else {
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}

void USB::USBD_StdDevReq(usbRequest req)
{

	uint8_t dev_addr;
	switch (req.mRequest & USB_REQ_TYPE_MASK)
	{
	case USB_REQ_TYPE_CLASS:
	case USB_REQ_TYPE_VENDOR:
		// pdev->pClass->Setup(pdev, req);
		break;

	case USB_REQ_TYPE_STANDARD:

		switch (req.Request)
		{
		case USB_REQ_GET_DESCRIPTOR:
			USBD_GetDescriptor(req);
			break;

		case USB_REQ_SET_ADDRESS:
			//USBD_SetAddress (pdev, req)
			dev_addr = (uint8_t)(req.Value) & 0x7FU;
			USBx_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);
			USBx_DEVICE->DCFG |= ((uint32_t)dev_addr << 4) & USB_OTG_DCFG_DAD;
			ep0_state = USBD_EP0_STATUS_IN;
			USB_EP0StartXfer(DIR_IN, 0, 0);
			break;

		case USB_REQ_SET_CONFIGURATION:
			//USBD_SetConfig (pdev, req);

			//if (dev_state == USBD_STATE_ADDRESSED) {
			dev_state = USBD_STATE_CONFIGURED;
			//USBD_CUSTOM_HID_Init
			USB_ActivateEndpoint(req.Value, true, USBD_EP_TYPE_INTR);		// Activate in endpoint
			USB_ActivateEndpoint(req.Value, false, USBD_EP_TYPE_INTR);		// Activate out endpoint

			//pdev->pClassData = USBD_malloc(sizeof (USBD_CUSTOM_HID_HandleTypeDef));

			//    (void)USB_EPStartXfer(hpcd->Instance, ep, (uint8_t)hpcd->Init.dma_enable);
			USB_EP0StartXfer(DIR_OUT, req.Value, 2);		// FIXME maxpacket is 2 for EP 1: CUSTOM_HID_EPIN_SIZE

			ep0_state = USBD_EP0_STATUS_IN;
			USB_EP0StartXfer(DIR_IN, 0, 0);

			break;

		case USB_REQ_GET_CONFIGURATION:
			// USBD_GetConfig (pdev, req);
			break;

		case USB_REQ_GET_STATUS:
			//USBD_GetStatus (pdev, req);
			break;

		case USB_REQ_SET_FEATURE:
			//USBD_SetFeature (pdev, req);
			break;

		case USB_REQ_CLEAR_FEATURE:
			//USBD_ClrFeature (pdev, req);
			break;

		default:
			//USBD_CtlError(pdev, req);
			break;
		}
		break;

		default:
			//USBD_CtlError(pdev, req);
			break;
	}

}

void USB::USB_EP0StartXfer(bool is_in, uint8_t epnum, uint32_t xfer_len)
{

	// IN endpoint
	if (is_in)
	{
		// Zero Length Packet?
		if (xfer_len == 0U) {
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);
			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
		} else {
			int maxPacket = (epnum == 0 ? ep0_maxPacket : ep_maxPacket);
			// Program the transfer size and packet count as follows: xfersize = N * maxpacket + short_packet pktcnt = N + (short_packet exist ? 1 : 0)
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);

			if (xfer_len > maxPacket) {		// currently set to 0x40
				xfer_rem = xfer_len - maxPacket;
				xfer_len = maxPacket;
			}

			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_PKTCNT & (1U << 19));
			USBx_INEP(epnum)->DIEPTSIZ |= (USB_OTG_DIEPTSIZ_XFRSIZ & xfer_len);
		}

		/* EP enable, IN data in FIFO */
		USBx_INEP(epnum)->DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);

		/* Enable the Tx FIFO Empty Interrupt for this EP */
		if (xfer_len > 0U) {
			USBx_DEVICE->DIEPEMPMSK |= 1UL << (epnum & EP_ADDR_MSK);
		}
	}
	else // OUT endpoint
	{
		// Program the transfer size and packet count as follows: pktcnt = N xfersize = N * maxpacket
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_XFRSIZ);
		USBx_OUTEP(epnum)->DOEPTSIZ &= ~(USB_OTG_DOEPTSIZ_PKTCNT);

/*
		if (xfer_len > 0U) {
			xfer_len = maxPacket;
		}
*/

		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & xfer_len);

		/* EP enable */
		USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
	}

}


void USB::USB_EPSetStall(uint8_t epnum) {
	if (((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == 0U) && (epnum != 0U)) {	//
		USBx_INEP(epnum)->DIEPCTL &= ~(USB_OTG_DIEPCTL_EPDIS);
	}
	USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

	// FIXME - cleared in USB_EP0StartXfer?
	//USB_EP0_OutStart
	USBx_OUTEP(0U)->DOEPTSIZ = 0U;			// USB_EP0_OutStart - set STUPCNT=3; PKTCNT=1; XFRSIZ=0x18
	USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
	USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
	USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;
}


bool USB::USB_ReadInterrupts(uint32_t interrupt){

	if (((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt && usbEventNo < 200) {
		usbEvents[usbEventNo] = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;
		usbEventNo++;
	}

	return ((USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) & interrupt) == interrupt;
}

void USB::SendReport(uint8_t *report, uint16_t len) {
	if (dev_state == USBD_STATE_CONFIGURED) {
		if (hid_state == CUSTOM_HID_IDLE) {
			hid_state = CUSTOM_HID_BUSY;
			outBuff = report;
			outBuffSize = len;
			USB_EP0StartXfer(DIR_IN, 1, len);
		}
	}
}

