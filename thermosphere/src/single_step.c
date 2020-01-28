/*
 * Copyright (c) 2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "single_step.h"
#include "core_ctx.h"
#include "sysreg.h"
#include "debug_log.h"

SingleStepState singleStepGetNextState(ExceptionStackFrame *frame)
{
    u64 mdscr = GET_SYSREG(mdscr_el1);
    bool mdscrSS = (mdscr & MDSCR_SS) != 0;
    bool pstateSS = (frame->spsr_el2 & PSTATE_SS) != 0;

    if (!mdscrSS) {
        return SingleStepState_Inactive;
    } else {
        return pstateSS ? SingleStepState_ActiveNotPending : SingleStepState_ActivePending;
    }
}

void singleStepSetNextState(ExceptionStackFrame *frame, SingleStepState state)
{
    u64 mdscr = GET_SYSREG(mdscr_el1);

    switch (state) {
        case SingleStepState_Inactive:
            // Unset mdscr_el1.ss
            mdscr &= ~MDSCR_SS;
            break;
        case SingleStepState_ActiveNotPending:
            // Set mdscr_el1.ss and pstate.ss
            mdscr |= MDSCR_SS;
            frame->spsr_el2 |= PSTATE_SS;
            break;
        case SingleStepState_ActivePending:
            // We never use this because pstate.ss is 0 by default...
            // Set mdscr_el1.ss and unset pstate.ss
            mdscr |= MDSCR_SS;
            frame->spsr_el2 &= ~PSTATE_SS;
            break;
        default:
            break;
    }

    SET_SYSREG(mdscr_el1, mdscr);
    __isb(); // TRM-mandated
}

void handleSingleStep(ExceptionStackFrame *frame, ExceptionSyndromeRegister esr)
{
    uintptr_t addr = frame->elr_el2;

    // Stepping range support;
    if (addr >= currentCoreCtx->steppingRangeStartAddr && addr < currentCoreCtx->steppingRangeEndAddr) {
        // Reactivate single-step
        singleStepSetNextState(frame, SingleStepState_ActiveNotPending);
    } else {
        // Disable single-step
        singleStepSetNextState(frame, SingleStepState_Inactive);
        // TODO report exception to gdb
    }

    DEBUG("Single-step exeception ELR = 0x%016llx, ISV = %u, EX = %u\n", frame->elr_el2, (esr.iss >> 24) & 1, (esr.iss >> 6) & 1);
}
