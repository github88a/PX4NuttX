/****************************************************************************
 * arch/arm/src/lpc313x/lpc313x_irq.c
 * arch/arm/src/chip/lpc313x_irq.c
 *
 *   Copyright (C) 2009 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <arch/irq.h>

#include "up_arch.h"
#include "os_internal.h"
#include "up_internal.h"

#include "lpc313x_intc.h"
#include "lpc313x_cgu.h"
#include "lpc313x_internal.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

uint32_t *current_regs;

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_irqinitialize
 ****************************************************************************/

void up_irqinitialize(void)
{
  int irq;

  /* Enable clock to interrupt controller */

  lpc313x_enableclock(CLKID_AHB2INTCCLK);  /*  AHB_TO_INTC_CLK */
  lpc313x_enableclock(CLKID_INTCCLK);      /*  INTC_CLK */

  /* Set the vector base. We don't use direct vectoring, so these are set to 0. */

  putreg32(0, LPC313X_INTC_VECTOR0);
  putreg32(0, LPC313X_INTC_VECTOR1);

  /* Set the priority treshold to 0, i.e. don't mask any interrupt on the
   * basis of priority level, for both targets (IRQ/FIQ)
   */

  putreg32(0, LPC313X_INTC_PRIORITYMASK0); /* Proc interrupt request 0: IRQ */
  putreg32(0, LPC313X_INTC_PRIORITYMASK1); /* Proc interrupt request 1: FIQ */
 
  /* Disable all interrupts. Start from index 1 since 0 is unused.*/

  for (irq = 1; irq <= NR_IRQS; irq++)
    {
      /* Initialize as high-active, disable the interrupt, set target to IRQ,
       * Set priority level to 1 (= lowest) for all the interrupt lines
       */

      uint32_t address = LPC313X_INTC_REQUEST(irq);
      putreg32(INTC_REQUEST_WEACTLOW|INTC_REQUEST_WEENABLE|INTC_REQUEST_TARGET_IRQ|
               INTC_REQUEST_PRIOLEVEL(1)|INTC_REQUEST_WEPRIO, address);

    }
}

/****************************************************************************
 * Name: up_disable_irq
 *
 * Description:
 *   Disable the IRQ specified by 'irq'
 *
 ****************************************************************************/

void up_disable_irq(int irq)
{
  /* Get the address of the request register corresponding to this
   * interrupt source
   */

  uint32_t address = LPC313X_INTC_REQUEST(irq);

  /* Clear the ENABLE bit with WE_ENABLE=1.  Configuration settings will be
   * preserved because WE_TARGET is zero.
   */

  putreg32(INTC_REQUEST_WEENABLE, address);
}

/****************************************************************************
 * Name: up_enable_irq
 *
 * Description:
 *   Enable the IRQ specified by 'irq'
 *
 ****************************************************************************/

void up_enable_irq(int irq)
{
  /* Get the address of the request register corresponding to this
   * interrupt source
   */

  uint32_t address = LPC313X_INTC_REQUEST(irq);

  /* Set the ENABLE bit with WE_ENABLE=1.  Configuration settings will be
   * preserved because WE_TARGET is zero.
   */

  putreg32(INTC_REQUEST_ENABLE|INTC_REQUEST_WEENABLE, address);
}

/****************************************************************************
 * Name: up_maskack_irq
 *
 * Description:
 *   Mask the IRQ and acknowledge it
 *
 ****************************************************************************/

void up_maskack_irq(int irq)
{
  /* Get the address of the request register corresponding to this
   * interrupt source
   */

  uint32_t address = LPC313X_INTC_REQUEST(irq);

  /* Clear the pending interrupt (INTC_REQUEST_CLRSWINT=1) AND disable interrupts
   * (ENABLE=0 && WE_ENABLE=1). Configuration settings will be preserved because
   * WE_TARGET is zero.
   */

  putreg32(INTC_REQUEST_CLRSWINT|INTC_REQUEST_WEENABLE, address);
}

/****************************************************************************
 * Name: up_prioritize_irq
 *
 * Description:
 *   Set the priority of an IRQ.
 *
 *   Since this API is not supported on all architectures, it should be
 *   avoided in common implementations where possible.
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_IRQPRIO
int up_prioritize_irq(int irq, int priority)
{
#warning "Not implemented"
  return OK;
}
#endif
