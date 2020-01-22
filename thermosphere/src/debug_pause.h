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

#pragma once

#include "utils.h"

void debugPauseSgiHandler(void);

// Hypervisor interrupts will be serviced during the pause-wait
void debugPauseWaitAndUpdateSingleStep(void);

// Note: these functions are not reentrant EXCEPT debugPauseCores(1 << currentCoreId)
// "Pause" makes sure all cores reaches the pause function before proceeding.
// "Unpause" doesn't synchronize, it just makes sure the core resumes & that "pause" can be called again.
void debugPauseCores(u32 coreList);
void debugUnpauseCores(u32 coreList, u32 singleStepList);
