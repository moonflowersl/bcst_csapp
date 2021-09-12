#include "memory/instruction.h"
#include "cpu/mmu.h"
#include "cpu/register.h"
#include "memory/dram.h"
#include <stdio.h>

static uint64_t decode_od(od_t od)
{
    if (od.type == IMM)
    {
        return *((uint64_t *)&od.imm);
    }
    else if (od.type == REG)
    {
        /*

        reg,rsp  0xabcd, 1  pmm[0xabcd] = 1

        pmm[0xabcd] = 0x1

        return 0xabcd

         */
        return (uint64_t)od.reg1;
    }
    else
    {
        // mm
        uint64_t vaddr = 0;
        if (od.type == MM_IMM)
        {
            vaddr = od.imm;
        } 
        else if (od.type == MM_REG)
        {
            // store reg
            vaddr = *(od.reg1);
        }
        else if (od.type == MM_IMM_REG)
        {
            vaddr = od.imm + *(od.reg1);
        }
        else if (od.type == MM_REG1_REG2)
        {
            vaddr = *(od.reg1) + *(od.reg2);
        }
        else if (od.type == MM_IMM_REG1_REG2)
        {
            vaddr = *(od.reg1) + *(od.reg2) + od.imm;
        }
        else if (od.type == MM_REG2_S)
        {
            vaddr = (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_IMM_REG2_S)
        {
            vaddr = od.imm + (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_REG1_REG2_S)
        {
            vaddr = (*(od.reg1)) + (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_IMM_REG1_REG2_S)
        {
            vaddr = od.imm + *(od.reg1) + (*(od.reg2)) * od.scal;
        }

        return vaddr;
    }
}

void instruction_cycle()
{
    inst_t *instr = (inst_t *)reg.rip;
    // inst_t instr = program[reg.rip];

    // imm: imm
    // reg: value
    // mm: paddr
    uint64_t src = decode_od(instr->src);
    uint64_t dst = decode_od(instr->dst);

    // add rax rbx
    // op = add_reg_reg = 3
    // handler_table[add_reg_reg] == handler_table[3] == add_reg_reg_handler

    handler_t handler = handler_table[instr->op]; // add_reg_reg_handler
    
    // add_reg_reg_handler(src = &rax, dst = &rbx)
    handler(src, dst);

    printf("    %s\n", instr->code);
}

void init_handler_table()
{
    handler_table[mov_reg_reg] = &mov_reg_reg_handler;
    handler_table[mov_reg_mem] = &mov_reg_mem_handler;
    handler_table[call] = &call_handler;
    handler_table[push_reg] = &push_reg_handler;
    handler_table[pop_reg] = &pop_reg_handler;
    handler_table[add_reg_reg] = &add_reg_reg_handler;
}


void mov_reg_reg_handler(uint64_t src, uint64_t dst)
{
    // src: reg
    // dst: reg
    *(uint64_t *)dst = *(uint64_t *)src;
    reg.rip = reg.rip + sizeof(inst_t);
}

void mov_reg_mem_handler(uint64_t src, uint64_t dst)
{
    // src: reg
    // dst: mem virutal address

    write64bits_dram(
        va2pa(dst),
        *(uint64_t *)src
    );

    reg.rip = reg.rip + sizeof(inst_t);
}

void push_reg_handler(uint64_t src, uint64_t dst)
{
    // src = reg
    // dst = empty
    reg.rsp = reg.rsp - 8; 
    write64bits_dram(
        va2pa(reg.rsp),
        *(uint64_t *)src
    );
    reg.rip = reg.rip + sizeof(inst_t);
}

void pop_reg_handler(uint64_t src, uint64_t dst)
{
    printf("pop");
}

void call_handler(uint64_t src, uint64_t dst)
{
    // src: imm address of call function
    reg.rsp = reg.rsp - 8;
    // write return address to rsp memory
    write64bits_dram(
        va2pa(reg.rsp),
        reg.rip + sizeof(inst_t)
    );
    reg.rip = src;
}

void add_reg_reg_handler(uint64_t src, uint64_t dst)
{
    // add_reg_reg_handler(src = &rax, dst = &rbx)
    /*
    rax pmm[0x1234] = 0x12340000
    rbx pmm[0x1235] = 0xabcd
    src: 0x1234
    dst: 0x1235
    *(uint64_t *)src 0x1234000
    *(uint64_t *)dst 0xabcd
    0x12340000 + 0xabcd = 0x1234abcd
    pmm[0x1235] = 0x1234abcd
    */
    *(uint64_t *)dst = *(uint64_t  *)dst + *(uint64_t *)src;
    reg.rip = reg.rip + sizeof(inst_t);
}