

void OTG_FS_IRQHandler(void)
{
	int epnum, ep_intr;

	// Handle spurious interrupt
	if (USB_ReadInterrupts() == 0)
		return;

	// Handle Mode mismatch interrupt
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_MMIS == USB_OTG_GINTSTS_MMIS) {
		USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_MMIS;
	}

	// OUT endpoint interrupt
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_OEPINT == USB_OTG_GINTSTS_OEPINT) {
		epnum = 0;

		// Read the output endpoint interrupt register to ascertain which endpoint fired the interrupt
		ep_intr = ((USBx_DEVICE->DAINT &= USBx_DEVICE->DAINTMSK) & 0xFFFF0000U) >> 16;

		while (ep_intr != 0U)
		{
			if ((ep_intr & 0x1U) != 0U)
			{
				epint = USBx_OUTEP(epnum)->DOEPINT &= USBx_DEVICE->DOEPMSK;

				if ((epint & USB_OTG_DOEPINT_XFRC) == USB_OTG_DOEPINT_XFRC)			//  Transfer completed
				{
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_XFRC;				// Clear interrupt
					//(void)PCD_EP_OutXfrComplete_int(hpcd, epnum);					// Appears to lead to placeholder function, overidable in user program
				}

				if ((epint & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP)			// SETUP phase done
				{
					/* Class B setup phase done for previous decoded setup */
					//(void)PCD_EP_OutSetupPacket_int(hpcd, epnum); //>>  HAL_PCD_SetupStageCallback(hpcd) >> USBD_LL_SetupStage
					USBD_ParseSetupRequest(&pdev->request, psetup);

					pdev->ep0_state = USBD_EP0_SETUP;
					pdev->ep0_data_len = pdev->request.wLength;

					switch (pdev->request.bmRequest & 0x1FU)
					{
					case USB_REQ_RECIPIENT_DEVICE:
						USBD_StdDevReq (pdev, &pdev->request);
						break;

					case USB_REQ_RECIPIENT_INTERFACE:
						USBD_StdItfReq(pdev, &pdev->request);
						break;

					case USB_REQ_RECIPIENT_ENDPOINT:
						USBD_StdEPReq(pdev, &pdev->request);
						break;

					default:
						USBD_LL_StallEP(pdev, (pdev->request.bmRequest & 0x80U));
						break;
					}


					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_STUP;				// Clear interrupt
				}

				if ((epint & USB_OTG_DOEPINT_OTEPDIS) == USB_OTG_DOEPINT_OTEPDIS)	// OUT token received when endpoint disabled
				{
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPDIS;			// Clear interrupt
				}

				if ((epint & USB_OTG_DOEPINT_OTEPSPR) == USB_OTG_DOEPINT_OTEPSPR)	// Status Phase Received interrupt
				{
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_OTEPSPR;			// Clear interrupt
				}


				if ((epint & USB_OTG_DOEPINT_NAK) == USB_OTG_DOEPINT_NAK)			// OUT NAK interrupt
				{
					USBx_OUTEP(epnum)->DOEPINT = USB_OTG_DOEPINT_NAK;				// Clear interrupt
				}
			}
			epnum++;
			ep_intr >>= 1U;
		}
	}

	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_IEPINT == USB_OTG_GINTSTS_IEPINT)
	{
	  /* Read in the device interrupt bits */
	  ep_intr = USB_ReadDevAllInEpInterrupt(hpcd->Instance);

	  epnum = 0U;

	  while (ep_intr != 0U)
	  {
		if ((ep_intr & 0x1U) != 0U) /* In ITR */
		{
		  epint = USB_ReadDevInEPInterrupt(hpcd->Instance, (uint8_t)epnum);

		  if ((epint & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC)
		  {
			fifoemptymsk = (uint32_t)(0x1UL << (epnum & EP_ADDR_MSK));
			USBx_DEVICE->DIEPEMPMSK &= ~fifoemptymsk;

			CLEAR_IN_EP_INTR(epnum, USB_OTG_DIEPINT_XFRC);

			if (hpcd->Init.dma_enable == 1U)
			{
			  hpcd->IN_ep[epnum].xfer_buff += hpcd->IN_ep[epnum].maxpacket;
			}

			HAL_PCD_DataInStageCallback(hpcd, (uint8_t)epnum);


			if (hpcd->Init.dma_enable == 1U)
			{
			  /* this is ZLP, so prepare EP0 for next setup */
			  if ((epnum == 0U) && (hpcd->IN_ep[epnum].xfer_len == 0U))
			  {
				/* prepare to rx more setup packets */
				(void)USB_EP0_OutStart(hpcd->Instance, 1U, (uint8_t *)hpcd->Setup);
			  }
			}
		  }
		  if ((epint & USB_OTG_DIEPINT_TOC) == USB_OTG_DIEPINT_TOC)
		  {
			CLEAR_IN_EP_INTR(epnum, USB_OTG_DIEPINT_TOC);
		  }
		  if ((epint & USB_OTG_DIEPINT_ITTXFE) == USB_OTG_DIEPINT_ITTXFE)
		  {
			CLEAR_IN_EP_INTR(epnum, USB_OTG_DIEPINT_ITTXFE);
		  }
		  if ((epint & USB_OTG_DIEPINT_INEPNE) == USB_OTG_DIEPINT_INEPNE)
		  {
			CLEAR_IN_EP_INTR(epnum, USB_OTG_DIEPINT_INEPNE);
		  }
		  if ((epint & USB_OTG_DIEPINT_EPDISD) == USB_OTG_DIEPINT_EPDISD)
		  {
			CLEAR_IN_EP_INTR(epnum, USB_OTG_DIEPINT_EPDISD);
		  }
		  if ((epint & USB_OTG_DIEPINT_TXFE) == USB_OTG_DIEPINT_TXFE)
		  {
			(void)PCD_WriteEmptyTxFifo(hpcd, epnum);
		  }
		}
		epnum++;
		ep_intr >>= 1U;
	  }
	}

	// Handle Resume Interrupt
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_WKUINT == USB_OTG_GINTSTS_WKUINT)
	{
	  /* Clear the Remote Wake-up Signaling */
	  USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;

	  if (hpcd->LPM_State == LPM_L1)
	  {
		hpcd->LPM_State = LPM_L0;

		HAL_PCDEx_LPM_Callback(hpcd, PCD_LPM_L0_ACTIVE);
	  }
	  else
	  {
		HAL_PCD_ResumeCallback(hpcd);
	  }

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_WKUINT;
	}

	// Handle Suspend Interrupt
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_USBSUSP == USB_OTG_GINTSTS_USBSUSP)
	{
	  if ((USBx_DEVICE->DSTS & USB_OTG_DSTS_SUSPSTS) == USB_OTG_DSTS_SUSPSTS)
	  {
		  //USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);		// stores status of suspend pdev->dev_state  = USBD_STATE_SUSPENDED
		  USBx_PCGCCTL |= USB_OTG_PCGCCTL_STOPCLK;
	  }
	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBSUSP;
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

	/* Handle Reset Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_USBRST == USB_OTG_GINTSTS_USBRST)
	{
	  USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;
	  (void)USB_FlushTxFifo(hpcd->Instance, 0x10U);

	  for (i = 0U; i < hpcd->Init.dev_endpoints; i++)
	  {
		USBx_INEP(i)->DIEPINT = 0xFB7FU;
		USBx_INEP(i)->DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
		USBx_OUTEP(i)->DOEPINT = 0xFB7FU;
		USBx_OUTEP(i)->DOEPCTL &= ~USB_OTG_DOEPCTL_STALL;
	  }
	  USBx_DEVICE->DAINTMSK |= 0x10001U;

	  if (hpcd->Init.use_dedicated_ep1 != 0U)
	  {
		USBx_DEVICE->DOUTEP1MSK |= USB_OTG_DOEPMSK_STUPM |
								   USB_OTG_DOEPMSK_XFRCM |
								   USB_OTG_DOEPMSK_EPDM;

		USBx_DEVICE->DINEP1MSK |= USB_OTG_DIEPMSK_TOM |
								  USB_OTG_DIEPMSK_XFRCM |
								  USB_OTG_DIEPMSK_EPDM;
	  }
	  else
	  {
		USBx_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_STUPM |
								USB_OTG_DOEPMSK_XFRCM |
								USB_OTG_DOEPMSK_EPDM |
								USB_OTG_DOEPMSK_OTEPSPRM |
								USB_OTG_DOEPMSK_NAKM;

		USBx_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_TOM |
								USB_OTG_DIEPMSK_XFRCM |
								USB_OTG_DIEPMSK_EPDM;
	  }

	  /* Set Default Address to 0 */
	  USBx_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;

	  /* setup EP0 to receive SETUP packets */
	  (void)USB_EP0_OutStart(hpcd->Instance, (uint8_t)hpcd->Init.dma_enable,
							 (uint8_t *)hpcd->Setup);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_USBRST);
	}

	/* Handle Enumeration done Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_ENUMDNE == USB_OTG_GINTSTS_ENUMDNE)
	{
	  (void)USB_ActivateSetup(hpcd->Instance);
	  hpcd->Init.speed = USB_GetDevSpeed(hpcd->Instance);

	  /* Set USB Turnaround time */
	  (void)USB_SetTurnaroundTime(hpcd->Instance,
								  HAL_RCC_GetHCLKFreq(),
								  (uint8_t)hpcd->Init.speed);


	  HAL_PCD_ResetCallback(hpcd);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_ENUMDNE);
	}

	/* Handle RxQLevel Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_RXFLVL == USB_OTG_GINTSTS_RXFLVL)
	{
	  USB_MASK_INTERRUPT(hpcd->Instance, USB_OTG_GINTSTS_RXFLVL);

	  temp = USBx->GRXSTSP;

	  ep = &hpcd->OUT_ep[temp & USB_OTG_GRXSTSP_EPNUM];

	  if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) ==  STS_DATA_UPDT)
	  {
		if ((temp & USB_OTG_GRXSTSP_BCNT) != 0U)
		{
		  (void)USB_ReadPacket(USBx, ep->xfer_buff,
							   (uint16_t)((temp & USB_OTG_GRXSTSP_BCNT) >> 4));

		  ep->xfer_buff += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
		  ep->xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
		}
	  }
	  else if (((temp & USB_OTG_GRXSTSP_PKTSTS) >> 17) ==  STS_SETUP_UPDT)
	  {
		(void)USB_ReadPacket(USBx, (uint8_t *)hpcd->Setup, 8U);
		ep->xfer_count += (temp & USB_OTG_GRXSTSP_BCNT) >> 4;
	  }
	  else
	  {
		/* ... */
	  }
	  USB_UNMASK_INTERRUPT(hpcd->Instance, USB_OTG_GINTSTS_RXFLVL);
	}

	/* Handle SOF Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_SOF == USB_OTG_GINTSTS_SOF)
	{

	  HAL_PCD_SOFCallback(hpcd);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SOF);
	}

	/* Handle Incomplete ISO IN Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_IISOIXFR == USB_OTG_GINTSTS_IISOIXFR)
	{
	  /* Keep application checking the corresponding Iso IN endpoint
	  causing the incomplete Interrupt */
	  epnum = 0U;


	  HAL_PCD_ISOINIncompleteCallback(hpcd, (uint8_t)epnum);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_IISOIXFR;
	}

	/* Handle Incomplete ISO OUT Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_PXFR_INCOMPISOOUT == USB_OTG_GINTSTS_PXFR_INCOMPISOOUT)
	{
	  /* Keep application checking the corresponding Iso OUT endpoint
	  causing the incomplete Interrupt */
	  epnum = 0U;


	  HAL_PCD_ISOOUTIncompleteCallback(hpcd, (uint8_t)epnum);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_PXFR_INCOMPISOOUT;
	}

	/* Handle Connection event Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_SRQINT == USB_OTG_GINTSTS_SRQINT)
	{

	  HAL_PCD_ConnectCallback(hpcd);

	  USB_OTG_FS->GINTSTS &= USB_OTG_GINTSTS_SRQINT;
	}

	/* Handle Disconnection event Interrupt */
	if (USB_ReadInterrupts() & USB_OTG_GINTSTS_OTGINT == USB_OTG_GINTSTS_OTGINT)
	{
	  temp = hpcd->Instance->GOTGINT;

	  if ((temp & USB_OTG_GOTGINT_SEDET) == USB_OTG_GOTGINT_SEDET)
	  {
		HAL_PCD_DisconnectCallback(hpcd);
	  }
	  hpcd->Instance->GOTGINT |= temp;
	}
}
