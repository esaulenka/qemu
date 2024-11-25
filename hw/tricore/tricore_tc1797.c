/*
 * TriCore Baseboard System emulation.
 *
 * Copyright (c) 2013-2014 Bastian Koppelmann C-Lab/University Paderborn
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */


#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "cpu.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "elf.h"
#include "qemu/error-report.h"


static void tricore_load_kernel(const char *kernel_filename)
{
    uint64_t entry;

    ssize_t kernel_size = 
        load_elf(kernel_filename, NULL,
                 NULL, NULL, &entry, NULL,
                 NULL, NULL, 0,
                 EM_TRICORE, 1, 0);
    if (kernel_size <= 0) {
        error_report("no kernel file '%s'", kernel_filename);
        exit(1);
    }
    TriCoreCPU *cpu = TRICORE_CPU(first_cpu);
    cpu->env.PC = entry;
}

static MemoryRegion * make_rom(const char *name, hwaddr base, hwaddr size)
{
    MemoryRegion *mr = g_new(MemoryRegion, 1);
    memory_region_init_rom(mr, NULL, name, size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), base, mr);
    return mr;
}

static MemoryRegion * make_ram(const char *name, hwaddr base, hwaddr size)
{
    MemoryRegion *mr = g_new(MemoryRegion, 1);
    memory_region_init_ram(mr, NULL, name, size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), base, mr);
    return mr;
}

static MemoryRegion * make_alias(const char *name, hwaddr base, 
                                 MemoryRegion *orig)
{
    MemoryRegion *mr = g_new(MemoryRegion, 1);
    memory_region_init_alias(mr, NULL, name, orig, 0, memory_region_size(orig));
    memory_region_add_subregion(get_system_memory(), base, mr);
    return mr;
}


static void tc1797_init(MachineState *machine)
{
    TriCoreCPU *cpu = TRICORE_CPU(cpu_create(machine->cpu_type));
    (void)cpu;

    MemoryRegion *pflash0_c = make_ram("PFLASH0", 0x80000000, 2 * MiB);
    MemoryRegion *pflash0_u = make_alias("PFLASH0_UNCACHED", 0xA0000000, pflash0_c);
    (void)pflash0_u;

    MemoryRegion *pflash1_c = make_ram("PFLASH1", 0x80200000, 2 * MiB);
    MemoryRegion *pflash1_u = make_alias("PFLASH1_UNCACHED", 0xA0200000, pflash1_c);
    (void)pflash1_u;

    MemoryRegion *dflash0_c = make_ram("DFLASH0", 0x8FE00000, 32 * KiB);
    MemoryRegion *dflash0_u = make_alias("DFLASH0_UNCACHED", 0xAFE00000, dflash0_c);
    (void)dflash0_u;

    MemoryRegion *dflash1_c = make_ram("DFLASH1", 0x8FE10000, 32 * KiB);
    MemoryRegion *dflash1_u = make_alias("DFLASH1_UNCACHED", 0xAFE10000, dflash1_c);
    (void)dflash1_u;

    MemoryRegion *olda_c = make_ram("OLDA", 0x8FE70000, 32 * KiB);
    MemoryRegion *olda_u = make_alias("OLDA_UNCACHED", 0xAFE70000, olda_c);
    (void)olda_u;

    MemoryRegion *brom_c = make_rom("BROM", 0x8FFFC000, 16 * KiB);
    MemoryRegion *brom_u = make_alias("BROM_UNCACHED", 0xAFFFC000, brom_c);
    (void)brom_u;

    MemoryRegion *spram = make_ram("SPRAM", 0xC0000000, 40 * KiB);
    MemoryRegion *spram_d = make_alias("SPRAM.D", 0xD4000000, spram);
    MemoryRegion *spram_e = make_alias("SPRAM.E", 0xE8500000, spram);
    (void)spram_d;(void)spram_e;

    MemoryRegion *ldram = make_ram("LDRAM", 0xD0000000, 128 * KiB);
    MemoryRegion *ldram_e = make_alias("LDRAM.E", 0xE8400000, ldram);
    (void)ldram_e;

    // TODO split into multiple areas
    MemoryRegion *periph = make_ram("REGS", 0xF0000000, 0x8800000);
    (void)periph;

    if (machine->kernel_filename) {
        tricore_load_kernel(machine->kernel_filename);
    }
}

static void tc1797_machine_init(MachineClass *mc)
{
    mc->desc = "TC1797";
    mc->init = tc1797_init;
    mc->default_cpu_type = TRICORE_CPU_TYPE_NAME("tc1797");
}

DEFINE_MACHINE("tricore_tc1797", tc1797_machine_init)
