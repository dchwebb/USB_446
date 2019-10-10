#include <USB.h>

void USB::USBInterruptHandler() {
	/* startup sequence:
	40000000 USB_OTG_GINTSTS_SRQINT (Session request/new session detected) 30
	800 USB_OTG_GINTSTS_USBSUSP (USB suspend) 11
	80000000 USB_OTG_GINTSTS_WKUINT (Resume/remote wakeup detected) 31
	1000 USB_OTG_GINTSTS_USBRST (USB reset) 12
	2000 USB_OTG_GINTSTS_ENUMDNE (Enumeration done) 13
	8 ... USB_OTG_GINTSTS_SOF (SOF) 3
	10 USB_OTG_GINTSTS_RXFLVL (RxFIFO non-empty)		receives Setup in packet
	10
	80000 OEPINT (OUT endpoint interrupt) 19
	40000 IEPINT (IN endpoint interrupt)  				USB_OTG_DIEPINT_TXFE
	40000 IEPINT (IN endpoint interrupt)  				USB_OTG_DIEPINT_XFRC
	80000
	10
	10			Address setup happens here but is not dealt with at this stage
	80000
	10
	10
	80000 OEPINT (OUT endpoint interrupt) 19
	40000 IEPINT (IN endpoint interrupt) 18
	10
	10
	80000 OEPINT (OUT endpoint interrupt) 19
	40000 IEPINT (IN endpoint interrupt) 18

	 */

	int epnum, ep_intr, epint;

	//int interruptCode = USB_ReadInterrupts();

	if (usbEventNo < 100) {
		// if (temp_gintsts & temp_gintmsk != 0x8) {
		usbEvents[usbEventNo] = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;


		if (usbEvents[usbEventNo] == 0x10) {
			eventOcc++;
		}
		if (usbEvents[usbEventNo] == 0x40000) {
			int susp = 1;
		}
		usbEventNo++;


	} else {
		int susp = 1;
	}

	// Handle spurious interrupt
	if (USB_ReadInterrupts() == 0)
		return;

	// Handle Mode mismatch interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_MMIS) == USB_OTG_GINTSTS_MMIS) {
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_MMIS;
	}

	/////////////////////////////////////////////// 80000 OEPINT
	// OUT endpoint interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_OEPINT) == USB_OTG_GINTSTS_OEPINT) {

		// Read the output endpoint interrupt register to ascertain which endpoint(s) fired an interrupt
		ep_intr = ((USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFF0000U) >> 16; // FIXME mask unnecessary with shift right?

		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0) {
			if ((ep_intr & 1) != 0) {
				epint = USBx_OUTEP(epnum)->DOEPINT & USBx_DEVICE->DOEPMSK;

				if ((epint & USB_OTG_DOEPINT_XFRC) == USB_OTG_DOEPINT_XFRC) {		// Transfer completed
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_XFRC;				// Clear interrupt
					//(void)PCD_EP_OutXfrComplete_int(hpcd, epnum);					// Appears to lead to placeholder function, overidable in user program
					//if (pdev->ep0_state == USBD_EP0_STATUS_OUT)
			        // STATUS PHASE completed, update ep0_state to idle
			        // pdev->ep0_state = USBD_EP0_IDLE;
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

					switch (req.mRequest & 0x1F)		// originally USBD_LL_SetupStage
					{
					case USB_REQ_RECIPIENT_DEVICE:
						//initially USB_REQ_GET_DESCRIPTOR >> USB_DESC_TYPE_DEVICE (bmrequest is 0x6)
						USBD_StdDevReq(req);
						break;

					case USB_REQ_RECIPIENT_INTERFACE:
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
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_IEPINT) == USB_OTG_GINTSTS_IEPINT)
	{
		// Read in the device interrupt bits [initially 1]
		ep_intr = (USBx_DEVICE->DAINT & USBx_DEVICE->DAINTMSK) & 0xFFFFU;

		// process each endpoint in turn incrementing the epnum and checking the interrupts (ep_intr) if that endpoint fired
		epnum = 0;
		while (ep_intr != 0U) {
			if ((ep_intr & 0x1) != 0) { // In ITR [initially true]
				// epint = USB_ReadDevInEPInterrupt(hpcd->Instance, (uint8_t)epnum);

				// [initially 0x80]
				epint = USBx_INEP((uint32_t)epnum)->DIEPINT & (USBx_DEVICE->DIEPMSK | (((USBx_DEVICE->DIEPEMPMSK >> (epnum & EP_ADDR_MSK)) & 0x1U) << 7));

				if ((epint & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC) {
					uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
					USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

					USBx_INEP(epnum)->DIEPINT = USB_OTG_DIEPINT_XFRC;

					//HAL_PCD_DataInStageCallback(hpcd, (uint8_t)epnum);

					if (((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_EPENA) == 0U) && (epnum != 0U)) {	//USB_EPSetStall
						USBx_INEP(epnum)->DIEPCTL &= ~(USB_OTG_DIEPCTL_EPDIS);
					}
					USBx_INEP(epnum)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;

					// FIXME - cleared in USB_EP0StartXfer?
					USBx_OUTEP(0U)->DOEPTSIZ = 0U;			// USB_EP0_OutStart - set STUPCNT=3; PKTCNT=1; XFRSIZ=0x18
					USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
					USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);
					USBx_OUTEP(0U)->DOEPTSIZ |=  USB_OTG_DOEPTSIZ_STUPCNT;

					//HAL_PCD_EP_Receive
					xfer_buff[0] = 0;
					//xfer_len = 0;
					xfer_count = 0;
					if (epnum == 0) {
						USB_EP0StartXfer(false, 0, nullptr, outBuffSize);
					} else {
						//USB_EPStartXfer(false, epnum, nullptr, outBuffSize);
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

					uint32_t len, len32b;
					if (outCount > outBuffSize) {			// Error
						return;
					}

					len = outBuffSize - outCount;		// outBuffSize is number of bytes to transfer; outCount is number transferred
					if (len > maxPacket)	len = maxPacket;
					len32b = (len + 3U) / 4U;

					while (((USBx_INEP(epnum)->DTXFSTS & USB_OTG_DTXFSTS_INEPTFSAV) >= len32b) && (outCount < outBuffSize) && (outBuffSize != 0U))
					{
						// Write to the FIFO
						len = outBuffSize - outCount;
						if (len > maxPacket)	len = maxPacket;
						len32b = (len + 3U) / 4U;

						USB_WritePacket(outBuff, epnum, (uint16_t)len);

						outBuff  += len;
						outCount += len;
					}

					if (outBuffSize <= outCount) {
						uint32_t fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
						USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;
					}


				}
			}
			epnum++;
			ep_intr >>= 1U;
		}
	}

	/////////////////////////////////////////////// 80000000 USB_OTG_GINTSTS_WKUINT
	// Handle Resume Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_WKUINT) == USB_OTG_GINTSTS_WKUINT)
	{
		// Clear the Remote Wake-up Signaling
		USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_WKUINT;
	}

	/////////////////////////////////////////////// 800 USBSUSP
	// Handle Suspend Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_USBSUSP) == USB_OTG_GINTSTS_USBSUSP)
	{

		if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_SUSPSTS) == USB_OTG_DSTS_SUSPSTS)
		{
			//USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);		// stores status of suspend pdev->dev_state  = USBD_STATE_SUSPENDED
			USBx_PCGCCTL |= USB_OTG_PCGCCTL_STOPCLK;
		}
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBSUSP;
	}

	/////////////////////////////////////////////// 1000 USB_OTG_GINTSTS_USBRST
	// Handle Reset Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_USBRST) == USB_OTG_GINTSTS_USBRST)
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
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_ENUMDNE) == USB_OTG_GINTSTS_ENUMDNE)
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

		// Open EP0 OUT
		USB_ActivateEndpoint(0, false, 0);

		// Open EP0 IN
		USB_ActivateEndpoint(0, true, 0);

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_ENUMDNE;
	}

	/////////////////////////////////////////////// 10 RXFLVL
	// Handle RxQLevel Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_RXFLVL) == USB_OTG_GINTSTS_RXFLVL)
	{
		if (usbEventNo > 25) {
			int susp = 1;
		}
		USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;

		uint32_t temp = USB_OTG_FS->GRXSTSP;		//OTG receive status debug read/OTG status read and	pop registers (OTG_GRXSTSR/OTG_GRXSTSP) not shown in SFR

		// Get the endpoint number
		epnum = temp & USB_OTG_GRXSTSP_EPNUM;

		if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) == STS_DATA_UPDT) {
			if ((temp & USB_OTG_GRXSTSP_BCNT) != 0U)
			{
				USB_ReadPacket(xfer_buff, (temp & USB_OTG_GRXSTSP_BCNT) >> 4);

				//xfer_buff += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
				xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
			}
		}
		else if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) ==  STS_SETUP_UPDT) {
			USB_ReadPacket(xfer_buff, 8U);
			xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
		}

		USB_OTG_FS->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
	}

	/////////////////////////////////////////////// 8 SOF
	// Handle SOF Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_SOF) == USB_OTG_GINTSTS_SOF)
	{
		// Doesn't seem to do anything
		//HAL_PCD_SOFCallback(hpcd);

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SOF;
	}

	/* Handle Incomplete ISO IN Interrupt */
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_IISOIXFR) == USB_OTG_GINTSTS_IISOIXFR)
	{
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_IISOIXFR;
	}

	/* Handle Incomplete ISO OUT Interrupt */
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_PXFR_INCOMPISOOUT) == USB_OTG_GINTSTS_PXFR_INCOMPISOOUT)
	{
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_PXFR_INCOMPISOOUT;
	}

	/////////////////////////////////////////////// 40000000 SRQINT
	// Handle Connection event Interrupt
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_SRQINT) == USB_OTG_GINTSTS_SRQINT)
	{
		//HAL_PCD_ConnectCallback(hpcd);		// this doesn't seem to do anything

		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SRQINT;
	}

	/* Handle Disconnection event Interrupt */
	if ((USB_ReadInterrupts() & USB_OTG_GINTSTS_OTGINT) == USB_OTG_GINTSTS_OTGINT)
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
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_LPMINT == USB_OTG_GINTSTS_LPMINT)
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

	// PA8, PA10, PA11, PA12 (NB PA9 - VBUS uses default values)
	GPIOA->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1;					// 10: Alternate function mode
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12;		// 11: High speed
	GPIOA->AFR[1] |= (10 << 0) | (10 << 8) | (10 << 12) | (10 << 16);															// Alternate Function 10 is OTG_FS

	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;				// USB OTG FS clock enable
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;				// Enable system configuration clock: used to manage external interrupt line connection to GPIOs

	NVIC_SetPriority(OTG_FS_IRQn, 0);
	NVIC_EnableIRQ(OTG_FS_IRQn);

	// *********************  HAL_PCD_Init() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
	USB_OTG_FS->GAHBCFG &= ~USB_OTG_GAHBCFG_GINT;		// Disable global interrupts

	// *********************  USB_CoreInit in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;		// Select FS interface (embedded Phy)

	// *********************  USB_CoreReset() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	// Reset the USB Core (needed after USB clock settings change)
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U);
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;		// Core Soft Reset
	while ((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST);


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
	*((uint32_t *)USB_OTG_FS + USB_OTG_PCGCCTL_BASE) &= ~USB_OTG_PCGCCTL_STOPCLK;		// peripheral register not in header - should be something like OTG_FS_PWRCLK->FS_PCGCCTL

	USBx_DEVICE->DCFG &= ~USB_OTG_DCFG_PFIVL;			// default 00: 80% of the frame interval: Indicates time within frame at which application must be notified using end of periodic frame interrupt. This can be used to determine if all the isochronous traffic for that frame is complete.

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

	USBx_DEVICE->DIEPMSK &= ~(USB_OTG_DIEPMSK_TXFURM);	// NB bit does not appear to be shown in the SFR

	USB_OTG_FS->GINTMSK = 0U;							// Disable all interrupts.
	USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;					// Clear any pending interrupts
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;		// Enable the Global interrupt: Receive FIFO non-empty mask

	// Enable interrupts matching to the Device mode ONLY
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_USBRST |	// USB suspend; USB reset
			USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |					// Enumeration done; IN endpoints interrupt
			USB_OTG_GINTMSK_OEPINT   | USB_OTG_GINTMSK_IISOIXFRM |				// OUT endpoints interrupt; Incomplete isochronous IN transfer
			USB_OTG_GINTMSK_PXFRM_IISOOXFRM | USB_OTG_GINTMSK_WUIM;				// Incomplete isochronous OUT transfer; Resume/remote wakeup detected

	//USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_SOFM;								// Start of frame
	USB_OTG_FS->GINTMSK |= (USB_OTG_GINTMSK_SRQIM | USB_OTG_GINTMSK_OTGINT);	// Session request/new session detected; OTG interrupt

	// ********************* USB_DevDisconnect() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
	// As long as this bit is set, the host does not see that the device is connected, and the device does not receive signals on the USB.
	USBx_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;				// Soft disconnect (not needed: already set by RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN)
	//HAL_Delay(3U);

	// ********************* HAL_PCDEx_SetRxFiFo() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
	USB_OTG_FS->GRXFSIZ = 128;		 					// RxFIFO depth

	// ********************* HAL_PCDEx_SetTxFiFo() in /Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
	// OTG_FS non-periodic transmit FIFO size register (Device mode) (FS_GNPTXFSIZ_Device in SFR)
	USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = ((uint32_t)64 << USB_OTG_TX0FD_Pos) |		// Endpoint 0 TxFIFO depth
			((uint32_t)128 << USB_OTG_TX0FSA_Pos);								// Endpoint 0 transmit RAM start  address

    // Multiply Tx_Size by 2 to get higher performance
    USB_OTG_FS->DIEPTXF[0] = ((uint32_t)128 << USB_OTG_DIEPTXF_INEPTXFD_Pos) |	// IN endpoint TxFIFO depth
    		((uint32_t)192 << USB_OTG_DIEPTXF_INEPTXSA_Pos);  					// IN endpoint FIFO2 transmit RAM start address

    // *********************  USBD_RegisterClass() in /Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    // call back class struct declared in /Middlewares/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h
    // links to call back functions declared in Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c
	/*
    typedef struct _Device_cb
	{
		uint8_t  (*Init)             (struct _USBD_HandleTypeDef *pdev , uint8_t cfgidx);
		uint8_t  (*DeInit)           (struct _USBD_HandleTypeDef *pdev , uint8_t cfgidx);

		// Control Endpoints
		uint8_t  (*Setup)            (struct _USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req);
		uint8_t  (*EP0_TxSent)       (struct _USBD_HandleTypeDef *pdev );
		uint8_t  (*EP0_RxReady)      (struct _USBD_HandleTypeDef *pdev );

		// Class Specific Endpoints
		uint8_t  (*DataIn)           (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
		uint8_t  (*DataOut)          (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
		uint8_t  (*SOF)              (struct _USBD_HandleTypeDef *pdev);
		uint8_t  (*IsoINIncomplete)  (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
		uint8_t  (*IsoOUTIncomplete) (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);

		uint8_t  *(*GetHSConfigDescriptor)(uint16_t *length);
		uint8_t  *(*GetFSConfigDescriptor)(uint16_t *length);
		uint8_t  *(*GetOtherSpeedConfigDescriptor)(uint16_t *length);
		uint8_t  *(*GetDeviceQualifierDescriptor)(uint16_t *length);
    } USBD_ClassTypeDef;
    */

    // *********************  USBD_CUSTOM_HID_RegisterInterface() in Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c
    // Links report definition to USB class in *pReport which is the array defining the HID report structure; definition: USBD_CustomHID_fops_FS
	/*
	typedef struct _USBD_CUSTOM_HID_Itf
	{
		uint8_t                  *pReport;
		int8_t (* Init)          (void);
		int8_t (* DeInit)        (void);
		int8_t (* OutEvent)      (uint8_t event_idx, uint8_t state);
	} USBD_CUSTOM_HID_ItfTypeDef;
	*/

    // *********************  USB_DevConnect() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
    USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;

    // *********************  USB_EnableGlobalInt() in Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
    USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
}


void USB::USB_ActivateEndpoint(uint32_t epnum, bool is_in, uint8_t eptype)
{

	if (is_in) {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_IEPM & (uint32_t)(1UL << (epnum & EP_ADDR_MSK));

		if ((USBx_INEP(epnum)->DIEPCTL & USB_OTG_DIEPCTL_USBAEP) == 0U) {
			USBx_INEP(epnum)->DIEPCTL |= (maxPacket & USB_OTG_DIEPCTL_MPSIZ) |
					((uint32_t)eptype << 18) | (epnum << 22) |
					USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
					USB_OTG_DIEPCTL_USBAEP;
		}
	} else {
		USBx_DEVICE->DAINTMSK |= USB_OTG_DAINTMSK_OEPM & ((uint32_t)(1UL << (epnum & EP_ADDR_MSK)) << 16);

		if (((USBx_OUTEP(epnum)->DOEPCTL) & USB_OTG_DOEPCTL_USBAEP) == 0U) {
			USBx_OUTEP(epnum)->DOEPCTL |= (maxPacket & USB_OTG_DOEPCTL_MPSIZ) |
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

	for (int i = 0U; i < count32b; i++)
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

	switch (req.Value >> 8)
	{
	case USB_DESC_TYPE_DEVICE:
		//pbuf = pdev->pDesc->GetDeviceDescriptor(pdev->dev_speed, &len);
		outBuff = USBD_FS_DeviceDesc;
		outBuffSize = sizeof(USBD_FS_DeviceDesc);
		break;

	case USB_DESC_TYPE_CONFIGURATION:

		//pbuf   = (uint8_t *)pdev->pClass->GetFSConfigDescriptor(&len);
		pbuf[1] = USB_DESC_TYPE_CONFIGURATION;

	case USB_DESC_TYPE_STRING:
		switch ((uint8_t)(req.Value)) {
		case USBD_IDX_LANGID_STR:
			//pbuf = pdev->pDesc->GetLangIDStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_MFC_STR:
			//pbuf = pdev->pDesc->GetManufacturerStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_PRODUCT_STR:
			//pbuf = pdev->pDesc->GetProductStrDescriptor(pdev->dev_speed, &len);
			break;
		case USBD_IDX_SERIAL_STR:
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
			// USBD_CtlError(pdev , req);
			return;
#endif
		}
		break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			//USBD_CtlError(pdev , req);
			return;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			//USBD_CtlError(pdev , req);
			return;
		default:
			//USBD_CtlError(pdev , req);
			return;
	}

	if ((outBuffSize != 0U) && (req.Length != 0U)) {

		outBuffSize = std::min(outBuffSize, (uint32_t)req.Length);
		USB_EP0StartXfer(true, 0, outBuff, outBuffSize);
	}

	if (req.Length == 0U) {
		USB_EP0StartXfer(true, 0, nullptr, 0);
	}
}

void USB::USBD_StdDevReq (usbRequest req)
{
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
			//USBD_SetAddress (pdev, req);
			break;

		case USB_REQ_SET_CONFIGURATION:
			//USBD_SetConfig (pdev, req);
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

void USB::USB_EP0StartXfer(bool is_in, uint8_t epnum, uint8_t* xfer_buff, uint32_t xfer_len)
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
			// Program the transfer size and packet count as follows: xfersize = N * maxpacket + short_packet pktcnt = N + (short_packet exist ? 1 : 0)
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_XFRSIZ);
			USBx_INEP(epnum)->DIEPTSIZ &= ~(USB_OTG_DIEPTSIZ_PKTCNT);

			if (xfer_len > maxPacket)		// currently set to 0x40
				xfer_len = maxPacket;

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

		if (xfer_len > 0U) {
			xfer_len = maxPacket;
		}

		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));
		USBx_OUTEP(epnum)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_XFRSIZ & (maxPacket));

		/* EP enable */
		USBx_OUTEP(epnum)->DOEPCTL |= (USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA);
		int susp = 1;
	}

}


uint32_t  USB::USB_ReadInterrupts(){

	uint32_t ret = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;
	if (usbEventNo == 0 || (usbEventNo < 100 && ret != usbEvents[usbEventNo - 1])) {
		usbEvents[usbEventNo] = ret;
		usbEventNo++;
	}

  return USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;
}
