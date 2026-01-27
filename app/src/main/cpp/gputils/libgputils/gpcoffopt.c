/* GNU PIC coff optimizing functions
   Copyright (C) 2005
   Craig Franklin

   Copyright (C) 2015-2016 Molnár Károly

   Experimental removal optimization common code.
   Copyright (C) 2017 Gonzalo Pérez de Olaguer Córdoba <salo@gpoc.es>

   Experimental pagesel and banksel removal.
   Copyright (C) 2017 Gonzalo Pérez de Olaguer Córdoba <salo@gpoc.es>

   Experimental PCALLW removal.
   Copyright (C) 2017 Gonzalo Pérez de Olaguer Córdoba <salo@gpoc.es>

This file is part of gputils.

gputils is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

gputils is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with gputils; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "stdhdr.h"
#include "libgputils.h"

#define COPT_NULL                           0
#define COPT_BRA14E_CURR_PAGE               (1 << 0)
#define COPT_BRA14E_OTHER_PAGE              (1 << 1)
#define COPT_GOTO_CURR_PAGE                 (1 << 2)
#define COPT_GOTO_OTHER_PAGE                (1 << 3)
#define COPT_CALL_CURR_PAGE                 (1 << 4)
#define COPT_CALL_OTHER_PAGE                (1 << 5)
#define COPT_PAGESEL_CURR_PAGE              (1 << 6)
#define COPT_PAGESEL_OTHER_PAGE             (1 << 7)

#define COPT_BANKSEL                        (1 << 8)

/* Only PIC14E and PIC14EX. */
#define COPT_BRA14E_MASK                    (COPT_BRA14E_CURR_PAGE | COPT_BRA14E_OTHER_PAGE)

#define COPT_GOTO_MASK                      (COPT_GOTO_CURR_PAGE | COPT_GOTO_OTHER_PAGE)

#define COPT_CALL_MASK                      (COPT_CALL_CURR_PAGE | COPT_CALL_OTHER_PAGE)

/* Only PIC14E and PIC14EX. */
#define COPT_REL_BRANCH_CURR_PAGE_MASK      COPT_BRA14E_CURR_PAGE
#define COPT_REL_BRANCH_OTHER_PAGE_MASK     COPT_BRA14E_OTHER_PAGE
#define COPT_REL_BRANCH_MASK                COPT_BRA14E_MASK

#define COPT_ABS_BRANCH_CURR_PAGE_MASK      (COPT_GOTO_CURR_PAGE | COPT_CALL_CURR_PAGE)
#define COPT_ABS_BRANCH_OTHER_PAGE_MASK     (COPT_GOTO_OTHER_PAGE | COPT_CALL_OTHER_PAGE)
#define COPT_ABS_BRANCH_MASK                (COPT_ABS_BRANCH_CURR_PAGE_MASK | COPT_ABS_BRANCH_OTHER_PAGE_MASK)

#define COPT_BRANCH_CURR_PAGE_MASK          (COPT_REL_BRANCH_CURR_PAGE_MASK | COPT_ABS_BRANCH_CURR_PAGE_MASK)
#define COPT_BRANCH_OTHER_PAGE_MASK         (COPT_REL_BRANCH_OTHER_PAGE_MASK | COPT_ABS_BRANCH_OTHER_PAGE_MASK)
#define COPT_BRANCH_MASK                    (COPT_BRANCH_CURR_PAGE_MASK | COPT_BRANCH_OTHER_PAGE_MASK)

#define COPT_PAGESEL_MASK                   (COPT_PAGESEL_CURR_PAGE | COPT_PAGESEL_OTHER_PAGE)

/* Number of reloc_properties_t type in an array. */
#define RELOC_PIPE_LENGTH                   4

typedef struct reloc_properties {
  gp_reloc_t        *relocation;
  const gp_symbol_t *label;             /* If exists so label which is linked here to. */
  const insn_t      *instruction;       /* The actual instruction. */
  unsigned int       state;             /* For COPT_... constants. */
  gp_boolean         protected;

  uint32_t           target_page;
  uint32_t           reloc_page;

  uint32_t           reloc_byte_addr;
  uint32_t           reloc_insn_addr;

  uint32_t           reloc_byte_length;
  uint32_t           reloc_insn_length;

  uint32_t           ram_bank;
} reloc_properties_t;

static reloc_properties_t   reloc_pipe[RELOC_PIPE_LENGTH];

static gp_section_t       **section_array;
static unsigned int         num_sections;

static gp_symbol_t        **register_array;
static unsigned int         num_registers;

static gp_boolean           first_banksel = false;

/*------------------------------------------------------------------------------------------------*/

/* Remove any weak symbols in the object. */

void
gp_coffopt_remove_weak(gp_object_t *Object)
{
  gp_symbol_t *symbol;

  gp_debug("Removing weak symbols from \"%s\".", Object->filename);

  /* Search the symbol table for extern symbols. */
  symbol = Object->symbol_list.first;
  while (symbol != NULL) {
    if (gp_coffgen_is_external_symbol(symbol) && (!gp_coffgen_symbol_has_reloc(symbol, COFF_SYM_RELOC_ALL))) {
      gp_debug("  removed weak symbol \"%s\"", symbol->name);
      /* It is not allowed to deleted because the gplink/cod.c will need this. */
      gp_coffgen_move_reserve_symbol(Object, symbol);
    }

    symbol = symbol->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Remove any relocatable section that doesn't have a symbol pointed to by a relocation. */

void
gp_coffopt_remove_dead_sections(gp_object_t *Object, int Pass, gp_boolean Enable_cinit_wanings)
{
  gp_section_t *section;
  gp_section_t *section_next;
  gp_boolean    section_removed;

  do {
    section_removed = false;
    gp_debug("Removing dead sections pass %i.", Pass);

    gp_coffgen_check_relocations(Object, Enable_cinit_wanings);
    section = Object->section_list.first;
    while (section != NULL) {
      section_next = section->next;

      if (FlagIsClr(section->opt_flags, OPT_FLAGS_PROTECTED_SECTION)) {
        gp_debug("Removing section \"%s\".", section->name);
        /* It is not allowed to deleted because the gplink/cod.c will need these. */
        gp_coffgen_move_reserve_section_symbols(Object, section);
        /* It is not allowed to deleted because the gplink/cod.c will need this. */
        gp_coffgen_move_reserve_section(Object, section);
        section_removed = true;
      }
      section = section_next;
    }

    /* take another pass */
    ++Pass;
  } while (section_removed);
}


/*------------------------------------------------------------------------------------------------*/

/* Deletes all states from relocation pipe. */

static void
_reloc_pipe_clear(void)
{
  memset(reloc_pipe, 0, sizeof(reloc_pipe));
}

/*------------------------------------------------------------------------------------------------*/

/* Moves the contents of relocation tube forward or backward. */

static void
_reloc_pipe_shift(gp_boolean Forward)
{
  size_t i;

  if (Forward) {
    /* Moves forward.

       reloc_pipe[last - 1] --> reloc_pipe[last]  -- The oldest relocation.
         .
         .
         .
       reloc_pipe[first]    --> reloc_pipe[first+1]
       reloc_pipe[first]    <-- 0                 -- The current relocation. */
    for (i = RELOC_PIPE_LENGTH - 1; i > 0; --i) {
      memcpy(&reloc_pipe[i], &reloc_pipe[i - 1], sizeof(reloc_properties_t));
    }

    memset(&reloc_pipe[0], 0, sizeof(reloc_properties_t));
  }
  else {
    /* Moves backward.

       reloc_pipe[first + 1] --> reloc_pipe[first] -- The current relocation.
         .
         .
         .
       reloc_pipe[last]      --> reloc_pipe[last - 1]
       reloc_pipe[last]      <-- 0                 -- The oldest relocation. */
    for (i = 0; i < (RELOC_PIPE_LENGTH - 1); ++i) {
      memcpy(&reloc_pipe[i], &reloc_pipe[i + 1], sizeof(reloc_properties_t));
    }

    memset(&reloc_pipe[RELOC_PIPE_LENGTH - 1], 0, sizeof(reloc_properties_t));
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Deletes one state from the relocation pipe. */

static void
_reloc_pipe_delete_state(size_t State_index)
{
  assert(State_index < RELOC_PIPE_LENGTH);

  while (State_index < (RELOC_PIPE_LENGTH - 1)) {
    memcpy(&reloc_pipe[State_index], &reloc_pipe[State_index + 1], sizeof(reloc_properties_t));
    ++State_index;
  }

  memset(&reloc_pipe[RELOC_PIPE_LENGTH - 1], 0, sizeof(reloc_properties_t));
}

/*------------------------------------------------------------------------------------------------*/

/* Make page address from an instruction address. */

static uint32_t
_page_addr_from_insn_addr(proc_class_t Class, uint32_t Insn_addr)
{
  return gp_processor_page_addr(Class, Insn_addr);
}

/*------------------------------------------------------------------------------------------------*/

/* Make page address from an byte address. */

static uint32_t
_page_addr_from_byte_addr(proc_class_t Class, uint32_t Byte_addr)
{
  return gp_processor_page_addr(Class, gp_processor_insn_from_byte_c(Class, Byte_addr));
}

/*------------------------------------------------------------------------------------------------*/

/* Decrease relocation addresses in a given list. */

static void
_reloc_decrease_addresses(proc_class_t Class, gp_reloc_t *Relocation, uint32_t Relocation_page,
                          uint32_t Insn_offset, uint32_t Byte_offset)
{
  gp_reloc_t         *reloc;
  gp_symbol_t        *symbol;
  const gp_section_t *section;

  if (Relocation == NULL) {
    return;
  }

  reloc = Relocation;
  while (reloc != NULL) {
    if (reloc->address >= Byte_offset) {
      reloc->address -= Byte_offset;
      symbol  = reloc->symbol;
      section = symbol->section;

      /* Prevents the modification of symbols on the other pages. */
      if (FlagIsSet(section->flags, STYP_ROM_AREA) &&
          (_page_addr_from_insn_addr(Class, symbol->value) == Relocation_page)) {
          /* Prevents the multiple modifications of symbol. */
        if (FlagIsClr(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_MODULE)) {
          symbol->value -= Insn_offset;
          FlagSet(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_MODULE);
        }
      }
    }

    reloc = reloc->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

static void
_label_arrays_make(proc_class_t Class)
{
  gp_section_t *section;
  unsigned int  i;

  if ((section_array == NULL) || (num_sections == 0)) {
    return;
  }

  for (i = 0; i < num_sections; ++i) {
    section = section_array[i];
    section->num_labels  = 0;
    section->label_array = gp_symbol_make_label_array(section, Class->org_to_byte_shift,
                                                      &(section->num_labels));
  }
}

/*------------------------------------------------------------------------------------------------*/

static void
_label_arrays_free(void)
{
  gp_section_t *section;
  unsigned int  i;

  if ((section_array == NULL) || (num_sections == 0)) {
    return;
  }

  for (i = 0; i < num_sections; ++i) {
    section = section_array[i];

    if (section->label_array != NULL) {
      free(section->label_array);
      section->label_array = NULL;
    }

    section->num_labels  = 0;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Sets or clears section optimize flag in a given list. */

static void
_label_clear_opt_flag(void)
{
  gp_section_t *section;
  gp_symbol_t  *label;
  unsigned int  i;
  unsigned int  j;

  if ((section_array == NULL) || (num_sections == 0)) {
    return;
  }

  for (i = 0; i < num_sections; ++i) {
    section = section_array[i];
    for (j = 0; j < section->num_labels; ++j) {
      label = section->label_array[j];
      /* This will be modifiable. */
      FlagClr(label->opt_flags, OPT_FLAGS_GPCOFFOPT_MODULE);
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Decrease label addresses in a given list. */

static void
_label_array_decrease_addresses(proc_class_t Class, gp_section_t *Section, uint32_t Start_address,
                                uint32_t Insn_offset)
{
  unsigned int  i;
  gp_symbol_t  *label;

  for (i = 0; i < Section->num_labels; ++i) {
    label = Section->label_array[i];

    /* Prevents the multiple modifications of symbol. */
    if (label->value >= Start_address) {
      if (FlagIsClr(label->opt_flags, OPT_FLAGS_GPCOFFOPT_MODULE)) {
        label->value -= Insn_offset;
        FlagSet(label->opt_flags, OPT_FLAGS_GPCOFFOPT_MODULE);
      }
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Decrease section addresses in a given list. */

static void
_sections_decrease_start_address(proc_class_t Class, const gp_section_t *Section, uint32_t Insn_offset,
                                 uint32_t Byte_offset)
{
  gp_section_t  *section;
  gp_symbol_t   *symbol;
  unsigned int   i;
  uint32_t       byte_address;
  uint32_t       insn_address;
  gp_symvalue_t  value_prev;

  if ((section_array == NULL) || (num_sections < 1)) {
    return;
  }

  value_prev = 0;
  for (i = 0; i < num_sections; ++i) {
    section = section_array[i];
    /* Prevents the modification of sections on other pages. */
    if (section->address > Section->address) {
      /* We must not modify an absolute section. */
      if (FlagIsClr(section->flags, STYP_ABS)) {
        byte_address = section->address - Byte_offset;
        insn_address = gp_processor_insn_from_byte_c(Class, byte_address);
        gp_mem_b_move(section->data, section->address, byte_address, section->size);
        section->address = byte_address;

        symbol = section->symbol;
        if (symbol != NULL) {
          value_prev     = symbol->value;
          symbol->value -= Insn_offset;
          assert((gp_symvalue_t)insn_address == symbol->value);
        }

        _label_array_decrease_addresses(Class, section, value_prev, Insn_offset);
      }
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Decrease line number addresses in a given list. */

static void
_linenum_decrease_addresses(proc_class_t Class, gp_section_t *First_section,
                            uint32_t Relocation_page, uint32_t Start_address, uint32_t Byte_offset)
{
  gp_section_t *section;
  gp_linenum_t *linenum;

  section = First_section;
  while (section != NULL) {
    /* We must not modify an absolute section. */
    if (FlagIsClr(section->flags, STYP_ABS)) {
      linenum = section->line_number_list.first;
      while (linenum != NULL) {
        /* Prevents the modification of linenumbers on other pages. */
        if ((_page_addr_from_byte_addr(Class, linenum->address) == Relocation_page) &&
            (linenum->address >= Start_address)) {
          linenum->address -= Byte_offset;
        }
        linenum = linenum->next;
      }
    }
    section = section->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Destroys an instruction from data memory of given section. */

static void
_destroy_insn(proc_class_t Class, gp_section_t *Section, uint32_t Byte_address, uint32_t Byte_length,
              const char *Symbol_name)
{
  gp_mem_b_delete_area(Section->data, Byte_address, Byte_length);
  Section->size -= Byte_length;
}

/*------------------------------------------------------------------------------------------------*/

/* Destroys a Pagesel directive and updates all related addresses. */

static void
_destroy_insn_and_update_addr(proc_class_t Class, gp_section_t *First_section, gp_section_t *Section,
                              unsigned int Insn_index)
{
  unsigned int  i;
  uint32_t      start_page;
  uint32_t      byte_addr_curr;
  uint32_t      byte_length_curr;
  uint32_t      insn_addr_curr;
  uint32_t      insn_length_curr;
  uint32_t      byte_addr_next;
  const char   *sym_name;

  byte_addr_curr   = reloc_pipe[Insn_index].reloc_byte_addr;
  byte_length_curr = reloc_pipe[Insn_index].reloc_byte_length;
  insn_addr_curr   = reloc_pipe[Insn_index].reloc_insn_addr;
  insn_length_curr = reloc_pipe[Insn_index].reloc_insn_length;
  byte_addr_next   = byte_addr_curr + byte_length_curr;
  start_page       = reloc_pipe[Insn_index].reloc_page;
  sym_name         = (reloc_pipe[Insn_index].relocation->symbol != NULL) ?
                                        reloc_pipe[Insn_index].relocation->symbol->name : NULL;

  _destroy_insn(Class, Section, byte_addr_curr, byte_length_curr, sym_name);
  gp_symbol_delete_by_value(Section->label_array, &Section->num_labels, insn_addr_curr);

  gp_coffgen_del_linenum_by_address_area(Section, byte_addr_curr, byte_addr_next - 1);
  _linenum_decrease_addresses(Class, First_section, start_page, byte_addr_next, byte_length_curr);

  /* Enable modification of address only in program memory. */
  _label_clear_opt_flag();

  _sections_decrease_start_address(Class, Section, insn_length_curr, byte_length_curr);

  _reloc_decrease_addresses(Class, reloc_pipe[Insn_index].relocation->next, start_page, insn_length_curr,
                            byte_length_curr);

  gp_coffgen_del_reloc(Section, reloc_pipe[Insn_index].relocation);

  /* Decrease the address of instruction in newer (younger) states. */
  for (i = 0; i < Insn_index; ++i) {
    reloc_pipe[i].reloc_byte_addr -= byte_length_curr;
    reloc_pipe[i].reloc_insn_addr -= insn_length_curr;
  }

  _reloc_pipe_delete_state(Insn_index);
}

/*------------------------------------------------------------------------------------------------*/

static gp_boolean
_insn_isReturn(proc_class_t Class, const gp_section_t *Section, unsigned int Byte_addr)
{
  uint16_t      data;
  const insn_t *instruction;

  if (Class->find_insn == NULL) {
    return false;
  }

  if (Class->i_memory_get(Section->data, Byte_addr, &data, NULL, NULL) != W_USED_ALL) {
    return false;
  }

  instruction = Class->find_insn(Class, data);
  if (instruction == NULL) {
    return false;
  }

  switch (instruction->icode) {
    case ICODE_RETFIE:
    case ICODE_RETI:
    case ICODE_RETIW:
    case ICODE_RETLW:
    case ICODE_RETP:
    case ICODE_RETURN:
      return true;

    default:
      return false;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* Analyze and add a new relocation state unto the relocation pipe. */

static void
_pagesel_reloc_analyze(proc_class_t Class, gp_section_t *Section, gp_reloc_t *Relocation,
                       unsigned int Num_pages)
{
  const gp_symbol_t *symbol;
  uint16_t           data;
  uint32_t           reloc_byte_addr;
  uint32_t           reloc_insn_addr;
  uint32_t           reloc_byte_length;
  uint32_t           value;
  uint32_t           reloc_page;
  uint32_t           target_page;

  symbol          = Relocation->symbol;
  reloc_byte_addr = Section->address        + Relocation->address;
  value           = (uint32_t)symbol->value + Relocation->offset;

  if (Class->i_memory_get(Section->data, reloc_byte_addr, &data, NULL, NULL) != W_USED_ALL) {
    gp_error("No instruction at 0x%0*X in program memory!", Class->addr_digits, reloc_byte_addr);
    assert(0);
  }

  reloc_insn_addr = gp_processor_insn_from_byte_c(Class, reloc_byte_addr);
  reloc_page      = gp_processor_page_addr(Class, reloc_insn_addr);
  target_page     = gp_processor_page_addr(Class, value);

  /* No relocation. */
  if ((reloc_pipe[1].relocation == NULL) ||
      /* A relocation which is not interesting. */
      (reloc_pipe[1].state == COPT_NULL) ||
      /* A relocation which is too far away. Meanwhile there is at least one other instruction. */
      ((reloc_pipe[1].reloc_insn_addr + reloc_pipe[1].reloc_insn_length) != reloc_insn_addr)) {
    /* Clears the contents of status pipe. */
    _reloc_pipe_clear();
  }

  reloc_pipe[0].relocation      = Relocation;
  reloc_pipe[0].label           = gp_symbol_find_by_value(Section->label_array, Section->num_labels, reloc_insn_addr);
  reloc_pipe[0].instruction     = (Class->find_insn != NULL) ? Class->find_insn(Class, data) : NULL;
  reloc_pipe[0].state           = COPT_NULL;
  reloc_pipe[0].protected       = ((reloc_pipe[0].label != NULL) && (reloc_pipe[0].label->reloc_count_all_section > 1)) ? true : false;
  reloc_pipe[0].target_page     = target_page;
  reloc_pipe[0].reloc_page      = reloc_page;
  reloc_pipe[0].reloc_byte_addr = reloc_byte_addr;
  reloc_pipe[0].reloc_insn_addr = reloc_insn_addr;

  reloc_byte_length = 0;

  switch (Relocation->type) {
    case RELOC_ALL:
      break;

    case RELOC_CALL:
      /* call function */
      reloc_pipe[0].state = (reloc_page == target_page) ? COPT_CALL_CURR_PAGE : COPT_CALL_OTHER_PAGE;
      reloc_byte_length   = 2;
      break;

    case RELOC_GOTO:
      /* goto label */
      reloc_pipe[0].state = (reloc_page == target_page) ? COPT_GOTO_CURR_PAGE : COPT_GOTO_OTHER_PAGE;
      reloc_byte_length   = 2;
      break;

    case RELOC_LOW:
      break;

    case RELOC_HIGH: {
      /* high(value) */
      if ((reloc_pipe[0].instruction != NULL) && (reloc_pipe[0].instruction->icode == ICODE_MOVLP)) {
        /* movlp high(value) */
        reloc_pipe[0].state = (reloc_page == target_page) ? COPT_PAGESEL_CURR_PAGE : COPT_PAGESEL_OTHER_PAGE;
      }
      reloc_byte_length = 2;
      break;
    }

    case RELOC_UPPER:
    case RELOC_P:
    case RELOC_BANKSEL:
    case RELOC_IBANKSEL:
    case RELOC_F:
    case RELOC_TRIS:
    case RELOC_TRIS_3BIT:
    case RELOC_MOVLR:
    case RELOC_MOVLB:
    case RELOC_GOTO2:
    case RELOC_FF1:
    case RELOC_FF2:
    case RELOC_LFSR1:
    case RELOC_LFSR2:
      break;

    case RELOC_BRA:
      /* bra label */
      reloc_pipe[0].state = (reloc_page == target_page) ? COPT_BRA14E_CURR_PAGE : COPT_BRA14E_OTHER_PAGE;
      reloc_byte_length   = 2;
      break;

    case RELOC_CONDBRA:
    case RELOC_ACCESS:
      break;

    case RELOC_PAGESEL_WREG:
      /* PIC12, PIC12E, PIC12I

       movlw value
       movwf STATUS

        OR

       PIC14

       movlw value
       movwf PCLATH */
      reloc_pipe[0].state = (reloc_page == target_page) ? COPT_PAGESEL_CURR_PAGE : COPT_PAGESEL_OTHER_PAGE;
      reloc_byte_length   = Class->pagesel_byte_length(Num_pages, true);
      break;

    case RELOC_PAGESEL_BITS:
      /* PIC12, PIC12E, PIC12I

       bcf STATUS, x
       bsf STATUS, x

        OR

       PIC14

       bcf PCLATH, x
       bsf PCLATH, x */
    case RELOC_PAGESEL_MOVLP:
      /* PIC14E, PIC14EX

       movlp value */
      reloc_pipe[0].state = (reloc_page == target_page) ? COPT_PAGESEL_CURR_PAGE : COPT_PAGESEL_OTHER_PAGE;
      reloc_byte_length   = Class->pagesel_byte_length(Num_pages, false);
      break;

    /* unimplemented relocations */
    case RELOC_PAGESEL:
    case RELOC_SCNSZ_LOW:
    case RELOC_SCNSZ_HIGH:
    case RELOC_SCNSZ_UPPER:
    case RELOC_SCNEND_LOW:
    case RELOC_SCNEND_HIGH:
    case RELOC_SCNEND_UPPER:
    case RELOC_SCNEND_LFSR1:
    case RELOC_SCNEND_LFSR2:
    default: {
        if (symbol->name != NULL) {
          gp_error("Unimplemented relocation = %s (%u) in section \"%s\" at symbol \"%s\".",
                   gp_coffgen_reloc_type_to_str(Relocation->type),
                   Relocation->type, Section->name, symbol->name);
        }
        else {
          gp_error("Unimplemented relocation = %s (%u) in section \"%s\".",
                   gp_coffgen_reloc_type_to_str(Relocation->type),
                   Relocation->type, Section->name);
        }
        assert(0);
      }
  }

  reloc_pipe[0].reloc_byte_length = reloc_byte_length;
  reloc_pipe[0].reloc_insn_length = gp_processor_insn_from_byte_c(Class, reloc_byte_length);
}

/*------------------------------------------------------------------------------------------------*/

/* If possible according to the rules, then removes a Pagesel directive. */

static gp_boolean
_pagesel_remove(proc_class_t Class, gp_section_t *First_section, gp_section_t *Section,
                gp_boolean Completion)
{
  unsigned int saturation;
  unsigned int byte_addr_next;

  saturation  = (reloc_pipe[0].relocation != NULL);
  saturation += (reloc_pipe[1].relocation != NULL);
  saturation += (reloc_pipe[2].relocation != NULL);
  saturation += (reloc_pipe[3].relocation != NULL);

  if (saturation == 0) {
    /* The State Pipe is empty. */
    return false;
  }

  if (Completion) {
    /* This is the last relocation on chain (a code section). */
    if ((reloc_pipe[0].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[0].protected)) {
      byte_addr_next = reloc_pipe[0].reloc_byte_addr + reloc_pipe[0].reloc_byte_length;

      if (_insn_isReturn(Class, Section, byte_addr_next)) {
        /*
          reloc_pipe[0]   pagesel current_page  <-- UNNECESSARY if not PROTECTED
          byte_addr_next: return (or these: retfie, retlw, reti, retp)
        */
        _destroy_insn_and_update_addr(Class, First_section, Section, 0);
        _reloc_pipe_shift(false);
        return true;
      }
    }
  }

  if (saturation >= 2) {
    /* The saturation of State Pipe at least 1/2. */
    if ((reloc_pipe[1].state == COPT_CALL_CURR_PAGE) &&
        (reloc_pipe[0].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[0].protected)) {
      /*
        reloc_pipe[1] call    function_on_current_page
        reloc_pipe[0] pagesel current_page  <--------- UNNECESSARY if not PROTECTED
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 0);
    }
    else if ((reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[0].state == COPT_PAGESEL_OTHER_PAGE)) {
      /*
        reloc_pipe[1] pagesel current_page  <------- UNNECESSARY if not PROTECTED
        reloc_pipe[0] pagesel other_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
    else if ((reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[0].state & COPT_ABS_BRANCH_CURR_PAGE_MASK)) {
      /*
        reloc_pipe[1] pagesel current_page  <------ UNNECESSARY if not PROTECTED
        reloc_pipe[0] goto    label_on_current_page

          OR

        reloc_pipe[1] pagesel current_page  <--------- UNNECESSARY if not PROTECTED
        reloc_pipe[0] call    function_on_current_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
    else if ((reloc_pipe[1].state & COPT_PAGESEL_MASK) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[0].state & COPT_REL_BRANCH_MASK)) {
      /*
        The 'bra' is a relative jump, no need to Pagesel.

        reloc_pipe[1] pagesel current_or_other_page  <----- UNNECESSARY if not PROTECTED
        reloc_pipe[0] bra     label_on_current_or_other_page
      */
      gp_warning("Strange relocation = %s (%u) with = %s (%u) in section \"%s\" at symbol \"%s\".",
                 gp_coffgen_reloc_type_to_str(reloc_pipe[1].relocation->type), reloc_pipe[1].relocation->type,
                 gp_coffgen_reloc_type_to_str(reloc_pipe[0].relocation->type), reloc_pipe[0].relocation->type,
                 Section->name, reloc_pipe[0].relocation->symbol->name);
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
  } /* if (saturation >= 2) */

  if (saturation >= 3) {
    /* The saturation of State Pipe at least 3/4. */
    if ((reloc_pipe[2].state == COPT_CALL_OTHER_PAGE) &&
        (reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) &&
        (reloc_pipe[0].state == COPT_PAGESEL_CURR_PAGE)) {
      /*
        reloc_pipe[2] call    function_on_other_page
        reloc_pipe[1] pagesel current_page  <------- clear PROTECTED
        reloc_pipe[0] pagesel current_page  <------- set PROTECTED
      */
      reloc_pipe[1].protected = false;
      reloc_pipe[0].protected = true;
    }
    else if ((reloc_pipe[2].state == COPT_CALL_OTHER_PAGE) &&
             (reloc_pipe[1].state == COPT_PAGESEL_OTHER_PAGE) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[2].target_page == reloc_pipe[1].target_page) &&
             (reloc_pipe[0].state == COPT_CALL_OTHER_PAGE) &&
             (reloc_pipe[1].target_page == reloc_pipe[0].target_page)) {
      /*
        reloc_pipe[2] call    function_on_other_page
        reloc_pipe[1] pagesel other_page  <--------- UNNECESSARY if not PROTECTED
        reloc_pipe[0] call    function_on_other_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
    else if ((reloc_pipe[2].state == COPT_CALL_CURR_PAGE) &&
             (reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[0].state == COPT_PAGESEL_CURR_PAGE)) {
      /*
        reloc_pipe[2] call    function_on_current_page
        reloc_pipe[1] pagesel current_page  <--------- UNNECESSARY if not PROTECTED
        reloc_pipe[0] pagesel current_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
    else if ((reloc_pipe[2].state == COPT_CALL_CURR_PAGE) &&
             (reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[1].protected) &&
             (reloc_pipe[0].state == COPT_CALL_CURR_PAGE)) {
      /*
        reloc_pipe[2] call    function_on_current_page
        reloc_pipe[1] pagesel current_page  <--------- UNNECESSARY if not PROTECTED
        reloc_pipe[0] call    function_on_current_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 1);
    }
  } /* if (saturation >= 3) */

  if (saturation == 4) {
    /* The State Pipe is full. */
    if ((reloc_pipe[3].state == COPT_CALL_OTHER_PAGE) &&
        (reloc_pipe[2].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[2].protected) &&
        (reloc_pipe[1].state == COPT_PAGESEL_CURR_PAGE) &&
        (reloc_pipe[0].state == COPT_CALL_CURR_PAGE)) {
      /*
        reloc_pipe[3] call    function_on_other_page
        reloc_pipe[2] pagesel current_page  <------- UNNECESSARY if not PROTECTED
        reloc_pipe[1] pagesel current_page
        reloc_pipe[0] call    function_on_current_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 2);
    }
    else if ((reloc_pipe[3].state == COPT_CALL_OTHER_PAGE) &&
             (reloc_pipe[2].state == COPT_PAGESEL_CURR_PAGE) && (!reloc_pipe[2].protected) &&
             (reloc_pipe[1].state == COPT_PAGESEL_OTHER_PAGE) &&
             (reloc_pipe[0].state == COPT_CALL_OTHER_PAGE)) {
      /*
        reloc_pipe[3] call    function_on_other_page
        reloc_pipe[2] pagesel current_page  <------- UNNECESSARY if not PROTECTED
        reloc_pipe[1] pagesel other_page
        reloc_pipe[0] call    function_on_other_page
      */
      _destroy_insn_and_update_addr(Class, First_section, Section, 2);
    }
  } /* if (saturation == 4) */

  return true;
}

/*------------------------------------------------------------------------------------------------*/

/* Deletes the unnecessary Pagesel directives from an object. */

void
gp_coffopt_remove_unnecessary_pagesel(gp_object_t *Object)
{
  proc_class_t  class;
  gp_section_t *first_section;
  gp_section_t *section;
  gp_reloc_t   *reloc_curr;
  gp_reloc_t   *reloc_next;
  unsigned int  num_pages;
  unsigned int  i;

  class = Object->class;

  /* Only case of PIC12 and PIC14 families. */
  if ((class != PROC_CLASS_PIC12)  && (class != PROC_CLASS_PIC12E) &&
      (class != PROC_CLASS_PIC12I) && (class != PROC_CLASS_SX)     &&
      (class != PROC_CLASS_PIC14)  && (class != PROC_CLASS_PIC14E) &&
      (class != PROC_CLASS_PIC14EX)) {
    return;
  }

  section_array  = NULL;
  num_sections   = 0;
  register_array = NULL;
  num_registers  = 0;

  gp_debug("Removing unnecessary pagesel instructions.");
  _reloc_pipe_clear();
  num_pages     = gp_processor_num_pages(Object->processor);
  first_section = Object->section_list.first;

  section = first_section;
  while (section != NULL) {
    _reloc_pipe_clear();

    if (gp_coffgen_section_has_data(section)) {
      num_sections  = 0;
      section_array = gp_coffgen_make_section_array(Object, &num_sections,
                              gp_processor_page_addr(class, gp_processor_insn_from_byte_c(class, section->address)),
                              STYP_ROM_AREA);
      _label_arrays_make(class);

      if (section->label_array != NULL) {
        reloc_curr = section->relocation_list.first;
        if (reloc_curr != NULL) {
          i = 0;
          do {
            reloc_next = reloc_curr->next;
            _pagesel_reloc_analyze(class, section, reloc_curr, num_pages);
            reloc_curr = reloc_next;
            _pagesel_remove(class, first_section, section, (reloc_curr == NULL));
            _reloc_pipe_shift(true);
            ++i;
          } while (reloc_curr != NULL);
        }
      }

      _label_arrays_free();

      if (section_array != NULL) {
        free(section_array);
      }
    }

    section = section->next;
  } /* while (section != NULL) */
}

/*------------------------------------------------------------------------------------------------*/

/* Analyze and add a new relocation state unto the relocation pipe. */

static gp_boolean
_banksel_reloc_analyze(proc_class_t Class, pic_processor_t Processor, gp_section_t *Section,
                       gp_reloc_t *Relocation, unsigned int Num_pages)
{
  const gp_symbol_t *symbol;
  uint16_t           data;
  uint32_t           reloc_byte_addr;
  uint32_t           reloc_insn_addr;
  uint32_t           reloc_byte_length;
  uint32_t           reloc_page;
  uint32_t           value;
  uint32_t           ram_bank;
  gp_boolean         need_clear;
  gp_boolean         there_is_banksel;

  symbol          = Relocation->symbol;
  reloc_byte_addr = Section->address + Relocation->address;
  reloc_insn_addr = gp_processor_insn_from_byte_c(Class, reloc_byte_addr);
  value           = (uint32_t)symbol->value + Relocation->offset;

  reloc_page      = gp_processor_page_addr(Class, reloc_insn_addr);

  if (Class->i_memory_get(Section->data, reloc_byte_addr, &data, NULL, NULL) != W_USED_ALL) {
    gp_error("No instruction at 0x%0*X in program memory!", Class->addr_digits, reloc_byte_addr);
    assert(0);
  }

  reloc_byte_length = 0;
  ram_bank          = 0;
  need_clear        = false;
  there_is_banksel  = false;

  switch (Relocation->type) {
    case RELOC_ALL:
      break;

    case RELOC_CALL:
    case RELOC_GOTO:
      need_clear = true;
      break;

    case RELOC_LOW:
    case RELOC_HIGH:
    case RELOC_UPPER:
    case RELOC_P:
      break;

    case RELOC_BANKSEL:
      ram_bank          = gp_processor_bank_addr(Processor, value);
      reloc_byte_length = Class->banksel_byte_length(Num_pages, false);
      there_is_banksel  = true;
      break;

    case RELOC_IBANKSEL:
      break;

    case RELOC_F:
    case RELOC_TRIS:
    case RELOC_TRIS_3BIT:
    case RELOC_MOVLR:
      break;

    case RELOC_MOVLB:
      ram_bank          = gp_processor_bank_addr(Processor, value);
      reloc_byte_length = 2;
      there_is_banksel  = true;
      break;

    case RELOC_GOTO2:
      need_clear = true;
      break;

    case RELOC_FF1:
    case RELOC_FF2:
    case RELOC_LFSR1:
    case RELOC_LFSR2:
      break;

    case RELOC_BRA:
    case RELOC_CONDBRA:
      need_clear = true;
      break;

    case RELOC_ACCESS:
    case RELOC_PAGESEL_WREG:
    case RELOC_PAGESEL_BITS:
    case RELOC_PAGESEL_MOVLP:
      break;

    /* unimplemented relocations */
    case RELOC_PAGESEL:
    case RELOC_SCNSZ_LOW:
    case RELOC_SCNSZ_HIGH:
    case RELOC_SCNSZ_UPPER:
    case RELOC_SCNEND_LOW:
    case RELOC_SCNEND_HIGH:
    case RELOC_SCNEND_UPPER:
    case RELOC_SCNEND_LFSR1:
    case RELOC_SCNEND_LFSR2:
    default: {
        if (symbol->name != NULL) {
          gp_error("Unimplemented relocation = %s (%u) in section \"%s\" at symbol \"%s\".",
                   gp_coffgen_reloc_type_to_str(Relocation->type),
                   Relocation->type, Section->name, symbol->name);
        }
        else {
          gp_error("Unimplemented relocation = %s (%u) in section \"%s\".",
                   gp_coffgen_reloc_type_to_str(Relocation->type),
                   Relocation->type, Section->name);
        }
        assert(0);
      }
  }

  if (need_clear) {
    _reloc_pipe_clear();
    return false;
  }

  if (there_is_banksel) {
    _reloc_pipe_shift(true);

    reloc_pipe[0].relocation        = Relocation;
    reloc_pipe[0].label             = gp_symbol_find_by_value(Section->label_array, Section->num_labels, reloc_insn_addr);
    reloc_pipe[0].instruction       = (Class->find_insn != NULL) ? Class->find_insn(Class, data) : NULL;
    reloc_pipe[0].state             = COPT_BANKSEL;
    reloc_pipe[0].protected         = ((reloc_pipe[0].label != NULL) && (reloc_pipe[0].label->reloc_count_all_section > 1)) ? true : false;
    reloc_pipe[0].reloc_page        = reloc_page;
    reloc_pipe[0].reloc_byte_addr   = reloc_byte_addr;
    reloc_pipe[0].reloc_insn_addr   = reloc_insn_addr;
    reloc_pipe[0].reloc_byte_length = reloc_byte_length;
    reloc_pipe[0].reloc_insn_length = gp_processor_insn_from_byte_c(Class, reloc_byte_length);
    reloc_pipe[0].ram_bank          = ram_bank;

    if (!first_banksel) {
      /* This is the first Banksel directive of section. Absolutely must protect it. */
      reloc_pipe[0].protected = true;
      first_banksel           = true;
    }
    return true;
  }

  return false;
}

/*------------------------------------------------------------------------------------------------*/

/* If possible according to the rules, then removes a Pagesel directive. */

static gp_boolean
_banksel_remove(proc_class_t Class, gp_section_t *First_section, gp_section_t *Section)
{
  unsigned int saturation;

  saturation  = (reloc_pipe[0].relocation != NULL);
  saturation += (reloc_pipe[1].relocation != NULL);
  saturation += (reloc_pipe[2].relocation != NULL);
  saturation += (reloc_pipe[3].relocation != NULL);

  if (saturation == 0) {
    /* The State Pipe is empty. */
    return false;
  }

  if (saturation >= 2) {
    if ((reloc_pipe[1].state == COPT_BANKSEL) &&
        (reloc_pipe[0].state == COPT_BANKSEL) &&
        (reloc_pipe[1].ram_bank == reloc_pipe[0].ram_bank)) {
      if (!reloc_pipe[1].protected) {
        /*
          reloc_pipe[1] banksel Z <--------- UNNECESSARY if not PROTECTED
          reloc_pipe[0] banksel Z
        */
        _destroy_insn_and_update_addr(Class, First_section, Section, 1);
      }
      else if (!reloc_pipe[0].protected) {
        /*
          reloc_pipe[1] banksel Z
          reloc_pipe[0] banksel Z <--------- UNNECESSARY if not PROTECTED
        */
        _destroy_insn_and_update_addr(Class, First_section, Section, 0);
      }
    }
  } /* if (saturation >= 2) */

  return true;
}

/*------------------------------------------------------------------------------------------------*/

/* Deletes the unnecessary Banksel directives from an object. */

void
gp_coffopt_remove_unnecessary_banksel(gp_object_t *Object)
{
  proc_class_t     class;
  pic_processor_t  processor;
  gp_section_t    *first_section;
  gp_section_t    *section;
  gp_reloc_t      *reloc_curr;
  gp_reloc_t      *reloc_next;
  unsigned int     num_banks;
  unsigned int     i;
  gp_boolean       may_remove;

  class     = Object->class;
  processor = Object->processor;

  if ((class != PROC_CLASS_PIC12)   && (class != PROC_CLASS_PIC12E) &&
      (class != PROC_CLASS_PIC12I)  && (class != PROC_CLASS_SX)     &&
      (class != PROC_CLASS_PIC14)   && (class != PROC_CLASS_PIC14E) &&
      (class != PROC_CLASS_PIC14EX) && (class != PROC_CLASS_PIC16)  &&
      (class != PROC_CLASS_PIC16E)) {
    return;
  }

  section_array  = NULL;
  num_sections   = 0;
  register_array = NULL;
  num_registers  = 0;

  gp_debug("Removing unnecessary banksel instructions.");
  num_registers  = 0;
  register_array = gp_symbol_make_register_array(Object, &num_registers);

  if (register_array == NULL) {
    return;
  }

  _reloc_pipe_clear();
  num_banks     = gp_processor_num_banks(Object->processor);
  first_section = Object->section_list.first;

  section = first_section;
  while (section != NULL) {
    first_banksel = false;
    _reloc_pipe_clear();

    if (gp_coffgen_section_has_data(section)) {
      num_sections  = 0;
      section_array = gp_coffgen_make_section_array(Object, &num_sections,
                              gp_processor_page_addr(class, gp_processor_insn_from_byte_c(class, section->address)),
                              STYP_ROM_AREA);
      _label_arrays_make(class);
      reloc_curr = section->relocation_list.first;
      if (reloc_curr != NULL) {
        i = 0;
        while (reloc_curr != NULL) {
          reloc_next = reloc_curr->next;
          may_remove = _banksel_reloc_analyze(class, processor, section, reloc_curr, num_banks);

          if (may_remove) {
            _banksel_remove(class, first_section, section);
          }

          reloc_curr = reloc_next;
          ++i;
        }

      }

      _label_arrays_free();

      if (section_array != NULL) {
        free(section_array);
      }
    } /* if (gp_coffgen_section_has_data(section)) */

    section = section->next;
  } /* while (section != NULL) */

  free(register_array);
}

/*------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------
 *
 * Common data for the experimental removal optimizations.
 *
 *----------------------------------------------------------------*/

/* This is a configuration parameter. */
/* Exclude empty sections from removal optimization. Their addresses won't be updated. */
#define REMOVAL_EXCLUDE_EMPTY_SECTIONS	0

/* the section may be changed by the removal optimization code */
#define OPT_FLAGS_GPCOFFOPT_REMOVAL_SECTION_VALID	(1 << 3)

/* the symbol may be changed by the removal optimization code */
#define OPT_FLAGS_GPCOFFOPT_REMOVAL_SYMBOL_VALID	(1 << 4)

/* element types */
/* list elements with same address are sorted by type */
/* this is a bitmask to help select several types at once */
#define REMOVAL_TYPE_SECTION		0x0001	/* a section label */
#define REMOVAL_TYPE_LABEL		0x0002	/* any other label */
#define REMOVAL_TYPE_BANKSEL		0x0004	/* bank selection (banksel/movlb) */
#define REMOVAL_TYPE_PAGESEL		0x0008	/* page selection (pagesel/movlp) */
#define REMOVAL_TYPE_BRANCH		0x0010	/* relative jump */
#define REMOVAL_TYPE_GOTO		0x0020	/* absolute jump */
#define REMOVAL_TYPE_CALL		0x0040	/* absolute call */
#define REMOVAL_TYPE_PCALLW_STUB	0x0100	/* PCALLW stub */
#define REMOVAL_TYPE_PCALLW_INSN	0x0200	/* PCALLW instruction */
#define REMOVAL_TYPE_PCALLW_ADDR	0x0400	/* PCALLW address */

/* element flags used for different purposes */
/* this is a bitmask */
#define REMOVAL_FLAG_FIRST		0x0001	/* first element in the relocation list */
#define REMOVAL_FLAG_FIXED		0x0002	/* the element's state is fixed and never changes */
#define REMOVAL_FLAG_REMOVE		0x0004	/* the bank selection instruction should be removed */
#define REMOVAL_FLAG_INITIALIZED	0x0008	/* used for initialization */
#define REMOVAL_FLAG_REPAGED		0x0010	/* section page has been changed */
#define REMOVAL_FLAG_REPAGE_FAILED	0x0020	/* section page change has failed */

#define REMOVAL_STATE_UNDEF	((uint32_t)-1)	/* undefined page or bank state */

#define BANKSEL_MAX_BANKS	32	/* max number of banks this code can handle */
#define PAGESEL_MAX_PAGES	32	/* max number of pages this code can handle */

/* the same element structure is used for pagesel and banksel removal optimization */
/* the fields used and their meaning depend on the element's type */
typedef struct _removal_s _removal_t;
struct _removal_s {
  _removal_t   *prev;		/* pointer to the previous element in the list */
  _removal_t   *next;		/* pointer to the next element in the list */
  gp_reloc_t   *relocation;	/* pointer to the relocation (banksel/pagesel/branch/goto/call) */
  gp_section_t *section;	/* pointer to the section (section) */
				/* pointer to the relocation section (banksel/pagesel/branch/goto/call) */
  gp_symbol_t  *symbol;		/* pointer to the symbol (label) */
				/* pointer to the relocation symbol (banksel/pagesel/branch/goto/call) */
  _removal_t   *source;		/* pointer to the first source linked to the label (label) */
				/* pointer to the target's section (pagesel) */
				/* pointer to the next source linked to the same label (branch/goto/call) */
  uint16_t      type;		/* type of the element */
  uint8_t       flags;		/* flags of the element */
  uint8_t       length;		/* instruction length  in bytes */
  uint32_t      address;	/* instruction address */
  uint32_t      page;		/* page of instruction address (used in sections only) */
  uint32_t      state;		/* state of the element */
  uint32_t      naddr;		/* type section: new address (pagesel code only) */
};

/* function that inserts a relocation in the list */
typedef gp_boolean _removal_add_reloc_t(gp_section_t *Section, gp_reloc_t *Reloc);

/* function that updates a relocation state */
typedef gp_boolean _removal_check_state_t(_removal_t *Ptr);

static _removal_t      *_removal_list;		/* the list of elements */
static gp_object_t     *_removal_object;	/* the object being processed */
static proc_class_t     _removal_class;		/* the processor class */
static pic_processor_t  _removal_processor;	/* the processor type */
static unsigned int     _banksel_banks;		/* actual number of banks */
static unsigned int     _pagesel_pages;		/* actual number of pages */

/*-----------------------------------------------------------------
 *
 * Common code for the experimental removal optimizations.
 *
 *----------------------------------------------------------------*/

/* Free all elements in the list. */
static void
_removal_free_list(void)
{
  if (_removal_list == NULL) {
    return;
  }

  _removal_list->prev->next = NULL;
  while (_removal_list != NULL) {
    _removal_t *ptr = _removal_list;
    _removal_list = ptr->next;
    free(ptr);
  }
}

/* Compare an element with an address and type. */
/* Return -1 (lesser), 0 (equal) or 1 (greater). */
static int
_removal_compare(_removal_t *Ptr, uint32_t Address, uint16_t Type)
{
  assert(Ptr != NULL);
  if (Ptr->address < Address) {
    return -1;
  }

  if (Ptr->address > Address) {
    return 1;
  }

  if (Ptr->type < Type) {
    return -1;
  }

  if (Ptr->type > Type) {
    return 1;
  }

  return 0;
}

/* Find an element in the list. */
/* Return the element if found, NULL otherwise. */
static _removal_t*
_removal_find_in_list(uint32_t Address, uint16_t Type)
{
  _removal_t *ptr;
  int         res;

  if (_removal_list == NULL) {
    return NULL;
  }

  ptr = _removal_list;
  while (((res = _removal_compare(ptr, Address, Type)) < 0) && (ptr->next != _removal_list)) {
    ptr = ptr->next;
  }

  return (res == 0) ? ptr : NULL;
}

/* Create a new element and insert it into the list. */
static _removal_t*
_removal_add_to_list(uint32_t Address, uint16_t Type)
{
  _removal_t *pos;
  _removal_t *ptr;
  int         res;

  /* create the new element */
  ptr = (_removal_t *)GP_Malloc(sizeof(_removal_t));
  assert(ptr != NULL);
  ptr->relocation = NULL;
  ptr->section = NULL;
  ptr->symbol = NULL;
  ptr->source = NULL;
  ptr->type = Type;
  ptr->flags = 0;
  ptr->length = 2;	/* default instruction length */
  ptr->address = Address;
  ptr->state = REMOVAL_STATE_UNDEF;

  if (_removal_list == NULL) {
    _removal_list = ptr;
    ptr->prev = ptr;
    ptr->next = ptr;
    return ptr;
  }

  pos = _removal_list;
  while (((res = _removal_compare(pos, Address, Type)) < 0) && (pos->next != _removal_list)) {
    pos = pos->next;
  }

  if (res > 0) {
    /* insert ptr before pos */
    ptr->prev = pos->prev;
    ptr->next = pos;
    /* ptr becomes the first item in the list */
    if (pos == _removal_list) {
      _removal_list = ptr;
    }
  }
  else {
    /* insert ptr after pos */
    ptr->prev = pos;
    ptr->next = pos->next;
  }

  ptr->prev->next = ptr;
  ptr->next->prev = ptr;
  return ptr;
}

/* Append a new section to the list. */
static _removal_t*
_removal_add_section(gp_section_t *Section)
{
  _removal_t *ptr;
  uint32_t    addr = gp_processor_insn_from_byte_c(_removal_class, Section->address);

  gp_debug ("      New section (addr:%#lx name:%s)", addr, Section->name);
  /* create the new element */
  ptr = _removal_add_to_list(addr, REMOVAL_TYPE_SECTION);
  ptr->section = Section;
  return ptr;
}

/* Append a new label to the list. */
/* Optionally link Source to the label. */
static _removal_t*
_removal_add_label(gp_symbol_t *Symbol, _removal_t *Source)
{
  _removal_t *ptr;

  /* Locate the label if it already exists */
  ptr = _removal_find_in_list(Symbol->value, REMOVAL_TYPE_LABEL);

  /* label not found: append a new one */
  if (ptr == NULL) {
    gp_debug("      New label (addr:%#lx section:%s name:%s)",
	Symbol->value, Symbol->section_name, Symbol->name);
    /* create the new element */
    ptr = _removal_add_to_list(Symbol->value, REMOVAL_TYPE_LABEL);
    ptr->symbol = Symbol;
  }

  /* link the source to the label */
  if (Source != NULL) {
    gp_debug("      New link (addr:%#lx from:%#lx)", ptr->address, Source->address);
    assert(Source->source == NULL);
    Source->source = ptr->source;
    ptr->source = Source;
  }
  return ptr;
}

/* Append a new relocation to the list. */
static _removal_t*
_removal_add_reloc(uint16_t Type, uint8_t Length, uint8_t Flags, gp_section_t *Section, gp_reloc_t *Reloc)
{
  _removal_t *ptr;
  uint32_t    address;

  assert(Section != NULL);
  assert(Reloc != NULL);
  assert(Reloc->symbol != NULL);

  address = gp_processor_insn_from_byte_c(_removal_class, Section->address + Reloc->address);

  gp_debug("    New relocation (addr:%#lx type:%hu len:%hu flags:%#hx section:%s symbol:%s)",
	address, Type, Length, Flags, Reloc->symbol->section_name, Reloc->symbol->name);

  /* create the new element */
  ptr = _removal_add_to_list (address, Type);
  ptr->relocation = Reloc;
  ptr->section = Section;
  ptr->symbol = Reloc->symbol;
  ptr->length = Length;
  ptr->flags = Flags;

  return ptr;
}

/* Build the list of elements needed for the removal optimization. */
/* Return true if there is something to remove. */
static gp_boolean
_removal_prepare_list(_removal_add_reloc_t *Add_reloc_ptr)
{
  gp_section_t *section;
  gp_reloc_t   *reloc;
  gp_boolean    ret;

  _removal_list = NULL;
  ret = false;

  /* First pass: identify sections */
  gp_debug("  Checking sections");
  section = _removal_object->section_list.first;
  while (section != NULL) {
    FlagClr(section->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SECTION_VALID);
    /* Accept only relocatable ROM sections. */
    if (FlagsIsAllClr(section->flags, STYP_ROM_AREA) || FlagIsSet(section->flags, STYP_ABS)) {
      gp_debug("    Rejecting section %s (addr:%#lx size:%lu flags:%#lx) (wrong flags)",
		section->name, section->address, section->size, section->flags);
    }
#if REMOVAL_EXCLUDE_EMPTY_SECTIONS
    else if (section->size == 0) {
      gp_debug("    Rejecting section %s (addr:%#lx size:%lu flags:%#lx) (empty section)",
		section->name, section->address, section->size, section->flags);
    }
#endif
    else {
      gp_debug("    Accepting section %s (addr:%#lx size:%lu flags:%#lx)",
		section->name, section->address, section->size, section->flags);
      FlagSet(section->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SECTION_VALID);
      /* add a section label */
      _removal_add_section (section);
    }
    section = section->next;
  }

  if (_removal_list == NULL) {
    gp_debug ("  No relocatable ROM sections found.");
    return false;
  }

  /* Second pass: identify relocations */
  section = _removal_object->section_list.first;
  while (section != NULL) {
    /* Only TEXT sections have relocations. */
    if (FlagIsSet(section->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SECTION_VALID) && FlagIsSet(section->flags, STYP_TEXT)) {
      assert(section->symbol != NULL);
      gp_debug ("  Looking for relocations in section %s (addr:%#lx)",
        	section->name, section->symbol->value);
      reloc = section->relocation_list.first;
      while (reloc != NULL) {
        uint16_t dummy;
        uint32_t addr = section->address + reloc->address;

	/* check that the relocation points to a valid instruction */
        if (_removal_class->i_memory_get(section->data, addr, &dummy, NULL, NULL) != W_USED_ALL) {
          gp_error("No instruction at 0x%0*X in program memory!", _removal_class->addr_digits, addr);
          assert(0);
        }

        switch (reloc->type) {
	  /* known and supported relocation types */
          case RELOC_ALL:
          case RELOC_CALL:
          case RELOC_GOTO:
          case RELOC_LOW:
          case RELOC_HIGH:
          case RELOC_UPPER:
          case RELOC_P:
          case RELOC_BANKSEL:
          case RELOC_IBANKSEL:
          case RELOC_F:
          case RELOC_TRIS:
          case RELOC_TRIS_3BIT:
          case RELOC_MOVLR:
          case RELOC_MOVLB:
          case RELOC_GOTO2:
          case RELOC_FF1:
          case RELOC_FF2:
          case RELOC_LFSR1:
          case RELOC_LFSR2:
          case RELOC_BRA:
          case RELOC_CONDBRA:
          case RELOC_ACCESS:
          case RELOC_PAGESEL_WREG:
          case RELOC_PAGESEL_BITS:
          case RELOC_PAGESEL_MOVLP:
            /* include the relocation in the list if needed */
            /* if it returns true the removal optimization must be tried */
            if ((*Add_reloc_ptr) (section, reloc))
              ret = true;
            break;

          /* unimplemented and unknown relocation types */
          case RELOC_PAGESEL:
          case RELOC_SCNSZ_LOW:
          case RELOC_SCNSZ_HIGH:
          case RELOC_SCNSZ_UPPER:
          case RELOC_SCNEND_LOW:
          case RELOC_SCNEND_HIGH:
          case RELOC_SCNEND_UPPER:
          case RELOC_SCNEND_LFSR1:
          case RELOC_SCNEND_LFSR2:
          default: {
              if (reloc->symbol->name != NULL) {
                gp_error("Unimplemented relocation = %s (%u) in section \"%s\" at symbol \"%s\".",
                         gp_coffgen_reloc_type_to_str(reloc->type),
                         reloc->type, section->name, reloc->symbol->name);
              }
              else {
                gp_error("Unimplemented relocation = %s (%u) in section \"%s\".",
                         gp_coffgen_reloc_type_to_str(reloc->type),
                         reloc->type, section->name);
              }
              assert(0);
            }
        }
        reloc = reloc->next;
      }
    }
    section = section->next;
  }

  if (!ret)
    /* Free the relocations list. */
    _removal_free_list();

  return ret;
}

/* Mark all the symbols that can be changed by the removal optimization code. */
static void
_removal_mark_symbols(void)
{
  gp_symbol_t *symbol;

  gp_debug("  Looking for symbols in ROM area");
  symbol = _removal_object->symbol_list.first;
  while (symbol != NULL) {
    FlagClr(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SYMBOL_VALID);
    if (symbol->section == NULL) {
      gp_debug("    Rejecting symbol %s (class:%u value:%#lx) (no section)",
    	symbol->name, symbol->class, symbol->value);
    }
    else if ((symbol->class != C_EXT) && (symbol->class != C_LABEL) && (symbol->class != C_SECTION)) {
      gp_debug("    Rejecting symbol %s (class:%u value:%#lx section:%s flags:%#lx) (wrong class)",
    	symbol->name, symbol->class, symbol->value, symbol->section->name, symbol->section->flags);
    }
    else if (FlagsIsAllClr(symbol->section->flags, STYP_ROM_AREA) || FlagIsSet(symbol->section->flags, STYP_ABS)) {
      gp_debug("    Rejecting symbol %s (class:%u value:%#lx section:%s flags:%#lx) (wrong section flags)",
    	symbol->name, symbol->class, symbol->value, symbol->section->name, symbol->section->flags);
    }
    else {
      gp_debug("    Accepting symbol %s (class:%u value:%#lx section:%s flags:%#lx)",
    	symbol->name, symbol->class, symbol->value, symbol->section->name, symbol->section->flags);
      FlagSet(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SYMBOL_VALID);
    }
    symbol = symbol->next;
  }
}

/* Identify the instructions that can be removed. */
/* Return true if there are instructions to remove. */
static gp_boolean
_removal_process_list (_removal_check_state_t *Check_state_ptr)
{
  _removal_t  *ptr;
  gp_boolean   changed;

  assert(_removal_list != NULL);

  /* mark the first element in the relocation list */
  _removal_list->flags |= REMOVAL_FLAG_FIRST;

  /* loop checking the relocation list until it is done */
  do {
    gp_debug ("  Starting removal optimization loop.");
    changed = false;
    ptr = _removal_list;
    do {
      /* elements with fixed state never change */
      if (((ptr->flags & REMOVAL_FLAG_FIXED) == 0) && (*Check_state_ptr) (ptr))
        changed = true;

      ptr = ptr->next;
    }
    while (ptr != _removal_list);
  }
  while (changed);

  /* now see if there are instructions to remove */
  ptr = _removal_list;
  do {
    if (ptr->flags & REMOVAL_FLAG_REMOVE)
	return true;

    ptr = ptr->next;
  }
  while (ptr != _removal_list);

  /* No instructions to remove. */
  /* Free the relocations list. */
  _removal_free_list();
  return false;
}

/* Update symbols and line numbers. */
static void
_removal_update(gp_section_t *Section, unsigned int Shift, uint32_t Start_address, uint32_t End_address)
{
  gp_linenum_t *linenum;
  gp_symbol_t  *symbol;

  if (Shift == 0)
    return;

  /* update line numbers */
  linenum = Section->line_number_list.first;
  while (linenum != NULL) {
    if ((linenum->address >= Start_address) && (linenum->address < End_address)) {
      gp_debug("    Changing linenumber %u (section:%s from:%#lx to:%#lx)",
    	linenum->line_number, Section->name, linenum->address, linenum->address - Shift);
      linenum->address -= Shift;
    }
    linenum = linenum->next;
  }

  /* update symbols */
  Shift = gp_processor_insn_from_byte_c(_removal_class, Shift);
  Start_address = gp_processor_insn_from_byte_c(_removal_class, Start_address);
  End_address = gp_processor_insn_from_byte_c(_removal_class, End_address);
  symbol = _removal_object->symbol_list.first;
  while (symbol != NULL) {
    if (FlagIsSet(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SYMBOL_VALID)
        && (symbol->value >= Start_address) && (symbol->value < End_address)) {
      gp_debug("    Changing symbol %s (section:%s from:%#lx to:%#lx)",
    	symbol->name, symbol->section_name, symbol->value, symbol->value - Shift);
      symbol->value -= Shift;
      /* done with this symbol */
      FlagClr(symbol->opt_flags, OPT_FLAGS_GPCOFFOPT_REMOVAL_SYMBOL_VALID);
    }
    symbol = symbol->next;
  }

}

/* Update a section and return the new shift value. */
static unsigned int
_removal_update_section (_removal_t *Ptr, unsigned int Shift, unsigned int Byte_count, unsigned int Insn_count)
{
  gp_section_t *section;
  uint32_t      isaddr;
  uint32_t      inaddr;
  uint32_t      csaddr;
  uint32_t      saddr;
  uint32_t      spage;
  uint32_t      eaddr;
  uint32_t      epage;

  section = Ptr->section;
  /* initial byte start address of the section */
  isaddr = section->address;
  /* initial byte address after the section */
  inaddr = isaddr + section->size;

  gp_debug("  Updating section %s (addr:%#lx size:%lu shift:%u bcnt:%u icnt:%u)",
           section->name, isaddr, section->size, Shift, Byte_count, Insn_count);

  /* instruction start and end addresses and pages if the section is moved and shrinked */
  saddr = gp_processor_insn_from_byte_c(_removal_class, isaddr - Shift);
  spage = gp_processor_page_addr(_removal_class, saddr);
  eaddr = gp_processor_insn_from_byte_c(_removal_class, isaddr - Shift + section->size - Byte_count - 2);
  epage = gp_processor_page_addr(_removal_class, eaddr);
  /* if moving the section as specified makes it cross a page boundary */
  /* move it to the start of the end page */
  if (spage != epage) {
    /* new shift to apply */
    Shift = isaddr - gp_processor_byte_from_insn_c(_removal_class, epage);
    /* new instruction start and end addresses and pages */
    saddr = spage = epage;
    eaddr = gp_processor_insn_from_byte_c(_removal_class, isaddr - Shift + section->size - Byte_count - 2);
  }

  /* now move the section */
  if (Shift != 0) {
    /* move section data and update its address and size */
    gp_debug("    Moving section %s (from:%#lx to:%#lx size:%lu)",
             section->name, isaddr, isaddr - Shift, section->size);
    gp_mem_b_move(section->data, isaddr, isaddr - Shift, section->size);
    section->address -= Shift;
    section->shadow_address -= Shift;
  }

  /* current byte start address for symbol and line number updates */
  csaddr = isaddr;

  /* now remove the instructions inside the section */
  while (Insn_count-- > 0) {

    uint32_t    rbsaddr;	/* relocation byte start address */
    uint32_t    rbshift;	/* relocation byte shift */
    uint32_t    rbnaddr;	/* relocation byte next address */
    uint32_t    risaddr;	/* relocation instruction start address */
    uint32_t    rishift;	/* relocation instruction shift */
    uint16_t    dummy;
    gp_reloc_t *reloc;

    /* point to the next instruction to remove */
    do
      Ptr = Ptr->next;
    while ((Ptr->flags & REMOVAL_FLAG_REMOVE) == 0);
    assert(Ptr->section == section);

    /* byte address of instruction to remove */
    rbsaddr = section->address + Ptr->relocation->address;
    /* number of bytes to remove */
    rbshift = Ptr->length;
    /* instruction address of instruction to remove */
    risaddr = gp_processor_insn_from_byte_c(_removal_class, rbsaddr);
    /* number of instructions to remove */
    rishift = gp_processor_insn_from_byte_c(_removal_class, rbshift);
    /* byte address after the instruction to remove */
    rbnaddr = rbsaddr + rbshift;

    gp_debug("    Removing instruction %#lx (addr:%#lx:%lu insn:%#lx:%lu next:%#lx)", Ptr->address,
             rbsaddr, rbshift, risaddr, rishift, rbnaddr);

    if (_removal_class->i_memory_get(section->data, rbsaddr, &dummy, NULL, NULL) != W_USED_ALL) {
      gp_error("No instruction at 0x%0*X in program memory!", _removal_class->addr_digits, rbsaddr);
      assert(0);
    }

    /* remove the instruction */
    gp_debug("    Changing size of section %s (addr:%#lx from:%lu to:%lu)",
             section->name, section->address, section->size, section->size - rbshift);
    gp_mem_b_delete_area(section->data, rbsaddr, rbshift);
    section->size -= rbshift;

    /* remove the line number */
    gp_coffgen_del_linenum_by_address_area(section, rbsaddr, rbnaddr - 1);

    /* update following relocation addresses in current section */
    reloc = section->relocation_list.first;
    while (reloc != NULL) {
      if (reloc->address > Ptr->relocation->address) {
        gp_debug("    Changing relocation address (section:%s symbol:%s from:%#lx to:%#lx)",
          	 reloc->symbol->section_name, reloc->symbol->name, reloc->address, reloc->address - rbshift);
        reloc->address -= rbshift;
      }
      reloc = reloc->next;
    }

    /* update symbols and line numbers */
    isaddr += rbshift;
    _removal_update(section, Shift, csaddr, isaddr + Ptr->relocation->address);
    csaddr = isaddr + Ptr->relocation->address;
    Shift += rbshift;

    /* remove the relocation */
    gp_coffgen_del_reloc(section, Ptr->relocation);

  }

  /* update symbols and line numbers for the rest of the section */
  _removal_update(section, Shift, csaddr, inaddr);
  return Shift;
}

/* Remove the unnecessary instructions. */
static void
_removal_remove_instructions (void)
{
  /* pointer to traverse the list */
  _removal_t   *ptr;
  /* pointer to the first element of the section */
  _removal_t   *first;
  /* number of bytes to decrease the start of the section */
  unsigned int  shift;
  /* number of bytes to remove inside the section */
  unsigned int  bcnt;
  /* number of instructions to remove inside the section */
  unsigned int  icnt;

  assert(_removal_list != NULL);
  assert(_removal_list->type == REMOVAL_TYPE_SECTION);

  /* now see which instructions can be removed */
  ptr = _removal_list;
  first = NULL;
  shift = 0;
  bcnt = 0;
  icnt = 0;
  do {
    if (ptr->type == REMOVAL_TYPE_SECTION) {
      /* here starts a new section */
      if (first != NULL) {
        shift = _removal_update_section(first, shift, bcnt, icnt);
      }

      first = ptr;
      bcnt = 0;
      icnt = 0;
    }
    else if (ptr->flags & REMOVAL_FLAG_REMOVE) {
      ++icnt;
      bcnt += ptr->length;
    }
    ptr = ptr->next;
  }
  while (ptr != _removal_list);
  /* update the last section */
  _removal_update_section(first, shift, bcnt, icnt);
}

/*------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------
 *
 * Exclusive code for experimental pagesel removal.
 *
 *----------------------------------------------------------------*/

/* Append to the list the relocation item if it is related to pagesel. */
/* Return true if pagesel removal optimization must be done. */
static gp_boolean
_pagesel_add_reloc(gp_section_t *Section, gp_reloc_t *Reloc)
{
  _removal_t    *ptr;
  uint32_t      addr;
  uint16_t      data;
  const insn_t *insn;
  uint16_t      type;
  uint8_t       length;
  uint8_t       flags;

  type = 0;
  length = 2;	/* default instruction length */
  flags = 0;

  /* Only some types of relocations are related to pagesel removal optimization. */
  switch (Reloc->type) {
    case RELOC_CALL:
      type = REMOVAL_TYPE_CALL;
      flags |= REMOVAL_FLAG_FIXED;
      break;

    case RELOC_GOTO:
      type = REMOVAL_TYPE_GOTO;
      break;

    case RELOC_BRA:
      type = REMOVAL_TYPE_BRANCH;
      break;

    case RELOC_HIGH:
      if (_removal_class->find_insn == NULL) {
        return false;
      }

      addr = Section->address + Reloc->address;
      if (_removal_class->i_memory_get(Section->data, addr, &data, NULL, NULL) != W_USED_ALL) {
        return false;
      }

      insn = _removal_class->find_insn(_removal_class, data);
      if ((insn == NULL) || (insn->icode != ICODE_MOVLP)) {
        return false;
      }

      /* movlp high(value) */
      type = REMOVAL_TYPE_PAGESEL;
      break;

    case RELOC_PAGESEL_WREG:
      type = REMOVAL_TYPE_PAGESEL;
      length = _removal_class->pagesel_byte_length(_pagesel_pages, true);
      break;

    case RELOC_PAGESEL_BITS:
    case RELOC_PAGESEL_MOVLP:
      type = REMOVAL_TYPE_PAGESEL;
      length = _removal_class->pagesel_byte_length(_pagesel_pages, false);
      break;
    default:
      return false;
  }
  ptr = _removal_add_reloc(type, length, flags, Section, Reloc);

  if (type == REMOVAL_TYPE_PAGESEL) {
    /* we need a label to track the target state */
    ptr->source = _removal_add_label (Reloc->symbol, NULL);
    /* now point to the target section */
    while (ptr->source->type != REMOVAL_TYPE_SECTION) {
      ptr->source = ptr->source->prev;
    }
    return true;
  }
  /* add a label for the relocation */
  _removal_add_label(Reloc->symbol, ptr);
  return false;
}

/* this function is missing in gpprocessor.h */
static int
gp_processor_page_num(pic_processor_t Processor, int Address)
{
  proc_class_t class = Processor->class;

  if ((class == &proc_class_pic14e) || (class == &proc_class_pic14ex)) {
    return (class->page_addr(Address) >> PIC14_PAGE_SHIFT);
  }
  else {
    return class->check_page(Address);
  }
}

/* Check the state of a relocation element. */
/* Return true if it has changed, false otherwise. */
static gp_boolean
_pagesel_check_state(_removal_t *Ptr)
{

  if (Ptr->type == REMOVAL_TYPE_SECTION) {
    gp_boolean  changed = false;
    _removal_t *tmp;
    uint32_t    size;
    uint32_t    isize;
    uint32_t    saddr;
    uint32_t    eaddr;
    uint32_t    siaddr;
    uint32_t    eiaddr;
    uint32_t    sipage;
    uint32_t    eipage;
    uint32_t    spage;
    uint32_t    state;
    uint32_t    naddr;
    uint32_t    niaddr;

    if ((Ptr->flags & REMOVAL_FLAG_INITIALIZED) == 0) {
      if (Ptr->flags & REMOVAL_FLAG_FIRST) {
        /* First section. */
        Ptr->source = NULL;
      }
      else {
        /* Locate the previous section. */
        Ptr->source = Ptr->prev;
        while (Ptr->source->type != REMOVAL_TYPE_SECTION) {
          Ptr->source = Ptr->source->prev;
        }
      }

      Ptr->flags |= REMOVAL_FLAG_INITIALIZED;
      Ptr->page = gp_processor_page_addr(_removal_class, Ptr->address);
      changed = true;
    }

    /* calculate new section size */
    /* decrease initial size by the length of all the instructions to be removed */
    size = Ptr->section->size;
    tmp = Ptr->next;
    while ((tmp != _removal_list) && (tmp->type != REMOVAL_TYPE_SECTION)) {
      if (tmp->flags & REMOVAL_FLAG_REMOVE) {
        size -= tmp->length;
      }
      tmp = tmp->next;
    }

    isize = gp_processor_insn_from_byte_c(_removal_class, size);
    saddr = (Ptr->source == NULL) ? Ptr->section->address : Ptr->source->naddr;
    /* convert byte addresses to instruction addresses and pages */
    siaddr = gp_processor_insn_from_byte_c(_removal_class, saddr);
    sipage = gp_processor_page_addr(_removal_class, siaddr);
    /* an attempt to repage the section has failed */
    if ((sipage != Ptr->page) && (Ptr->flags & REMOVAL_FLAG_REPAGED)) {
      Ptr->flags |= REMOVAL_FLAG_REPAGE_FAILED;
      sipage = siaddr = Ptr->page;
      saddr = gp_processor_byte_from_insn_c(_removal_class, siaddr);
    }

    eaddr = saddr + size - 2;
    eiaddr = gp_processor_insn_from_byte_c(_removal_class, eaddr);
    eipage = gp_processor_page_addr(_removal_class, eiaddr);
    /* section crosses page boundaries */
    if (eipage != sipage) {
      sipage = siaddr = eipage;
      saddr = gp_processor_byte_from_insn_c(_removal_class, siaddr);
      eaddr = saddr + size - 2;
      eiaddr = gp_processor_insn_from_byte_c(_removal_class, eaddr);
    }

    /* this function is missing in gpprocessor.h */
    spage = gp_processor_page_num(_removal_processor, siaddr);
    state = (1 << spage);
    naddr = saddr + size;
    niaddr = gp_processor_insn_from_byte_c(_removal_class, naddr);
    /* state involves state and next address */
    if (!changed && (state == Ptr->state) && (naddr == Ptr->naddr)) {
      return false;
    }

    Ptr->naddr = naddr;
    Ptr->state = state;
    if (sipage != Ptr->page) {
      Ptr->flags |= REMOVAL_FLAG_REPAGED;
    }

    gp_debug("    Changed section (addr:%#lx state:%#lx page:%#lx start:%#lx size:%lu next:%#lx)%s",
	Ptr->address, Ptr->state, spage, siaddr, isize, niaddr,
	  Ptr->flags & REMOVAL_FLAG_REPAGE_FAILED
	    ? " REPAGE FAILED"
	    : Ptr->flags & REMOVAL_FLAG_REPAGED
	      ? " REPAGED"
	      : "");
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_PAGESEL) {
    gp_boolean changed = false;
    /* pagesel state is the one of its target section */
    if (Ptr->source->state != Ptr->state) {
      Ptr->state = Ptr->source->state;
/* BUG:      changed = true; */
    }

    /* pagesel instructions can be removed
     * if previous state is already the desired one
     */
    if ((Ptr->prev->state == Ptr->state) != ((Ptr->flags & REMOVAL_FLAG_REMOVE) != 0)) {
      Ptr->flags ^= REMOVAL_FLAG_REMOVE;
      changed = true;
    }

    if (!changed) {
      return false;
    }

    gp_debug("    Changed pagesel (addr:%#lx state:%#lx action:%s)",
	Ptr->address, Ptr->state, Ptr->flags & REMOVAL_FLAG_REMOVE ? "remove" : "keep");
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_LABEL) {
    /* labels linked from other instructions
     * exit state is the combination of the exit state of the previous instruction
     * and the exit states of all the instructions linked to it
     */
    uint32_t entry = Ptr->prev->state;
    _removal_t *src = Ptr->source;
    while (src != NULL) {
      entry |= src->state;
      src = src->source;
    }

    if (entry == Ptr->state) {
      return false;
    }

    Ptr->state = entry;
    gp_debug("    Changed label (addr:%#lx state:%#lx)", Ptr->address, Ptr->state);
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_BRANCH) {
    /* branch instructions are relative jumps and need no pagesel
     * exit state is the exit state of the previous instruction
     * (in case there is a preceding conditional jump)
     */
    uint32_t entry = Ptr->prev->state;
    if (entry == Ptr->state) {
      return false;
    }

    Ptr->state = entry;
    gp_debug("    Changed branch (addr:%#lx state:%#lx)", Ptr->address, Ptr->state);
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_GOTO) {
    /* goto instructions are absolute jumps and need pagesel
     * exit state is the exit state of the previous instruction
     * (in case there is a preceding conditional jump)
     */
    uint32_t entry = Ptr->prev->state;
    if (entry == Ptr->state) {
      return false;
    }

    Ptr->state = entry;
    gp_debug("    Changed goto (addr:%#lx state:%#lx)", Ptr->address, Ptr->state);
    return true;
  }

  /* any other type is not required */
  Ptr->flags |= REMOVAL_FLAG_FIXED;
  return false;
}

/*------------------------------------------------------------------------------------------------*/

/* Deletes the unnecessary Pagesel directives from an object. */

void
gp_coffopt_remove_unnecessary_pagesel_experimental(gp_object_t *Object)
{
  _removal_object = Object;
  _removal_class = Object->class;

  /* Only case of PIC12 and PIC14 families. */
  if ((_removal_class != PROC_CLASS_PIC12)   && (_removal_class != PROC_CLASS_PIC12E) &&
      (_removal_class != PROC_CLASS_PIC12I)  && (_removal_class != PROC_CLASS_SX)     &&
      (_removal_class != PROC_CLASS_PIC14)   && (_removal_class != PROC_CLASS_PIC14E) &&
      (_removal_class != PROC_CLASS_PIC14EX)) {
    return;
  }

  _removal_processor = Object->processor;
  _pagesel_pages = gp_processor_num_pages(_removal_processor);

  gp_debug("Removing unnecessary pagesel instructions (class:%s processor:%s pages:%u)",
           gp_processor_class_to_str(_removal_class), gp_processor_name(_removal_processor,0), _pagesel_pages);
  assert(_pagesel_pages <= PAGESEL_MAX_PAGES);

  /* Build the list of all the relocation items related to pagesel. */
  /* Return true if there is something to remove. */
  if (!_removal_prepare_list(&_pagesel_add_reloc)) {
    gp_debug("  There are no pagesel instructions to remove.");
    return;
  }

  /* Mark all the symbols that can be changed by the pagesel removal optimization code. */
  _removal_mark_symbols();

  /* Identify the pagesel instructions that can be removed. */
  /* Return true if there are pagesel instructions to remove. */
  if (!_removal_process_list(&_pagesel_check_state)) {
    gp_debug("  There are no pagesel instructions that can be removed.");
    return;
  }

  /* Remove the unnecessary instructions. */
  _removal_remove_instructions();

  /* Free the relocations list. */
  _removal_free_list();

  /* Rebuild the cinit table. */
  gp_cofflink_add_cinit_section(Object);
}

/*------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------
 *
 * Exclusive code for experimental banksel removal.
 *
 *----------------------------------------------------------------*/

/* Append to the list the relocation item if it is related to banksel */
/* Return true if banksel removal optimization must be done */
static gp_boolean
_banksel_add_reloc(gp_section_t *Section, gp_reloc_t *Reloc)
{
  _removal_t *ptr;
  uint16_t    type;
  uint8_t     length;
  uint8_t     flags;

  type = 0;
  length = 2;	/* default instruction length */
  flags = 0;

  /* Only some types of relocations are related to banksel removal optimization. */
  switch (Reloc->type) {
    case RELOC_BANKSEL:
      type = REMOVAL_TYPE_BANKSEL;
      length = _removal_class->banksel_byte_length(_banksel_banks, false);
      break;

    case RELOC_MOVLB:
      type = REMOVAL_TYPE_BANKSEL;
      break;

    case RELOC_CALL:
      type = REMOVAL_TYPE_CALL;
      flags |= REMOVAL_FLAG_FIXED;
      break;

    case RELOC_GOTO:
    case RELOC_GOTO2:
    case RELOC_BRA:
    case RELOC_CONDBRA:
      /* there is no need to make differences between goto and bra */
      type = REMOVAL_TYPE_GOTO;
      break;

    default:
      return false;
  }

  ptr = _removal_add_reloc(type, length, flags, Section, Reloc);

  if (type == REMOVAL_TYPE_BANKSEL) {
    /* bank selection instruction: set bank state */
    uint32_t value;
    uint32_t bank;

    value = Reloc->symbol->value + Reloc->offset;
    bank = gp_processor_bank_num(_removal_processor, value);
    assert(bank <= _banksel_banks);
    ptr->state = (1 << bank);
    return true;
  }

  /* add a label for the relocation */
  _removal_add_label(Reloc->symbol, ptr);
  return false;
}

/* Check the state of a relocation element. */
/* Return true if it has changed, false otherwise. */
static gp_boolean
_banksel_check_state(_removal_t *Ptr)
{
  if (Ptr->type == REMOVAL_TYPE_BANKSEL) {
    /* banksel instructions can be removed
     * if previous state is already the desired one
     */
    if ((Ptr->prev->state == Ptr->state) == ((Ptr->flags & REMOVAL_FLAG_REMOVE) != 0)) {
      return false;
    }

    Ptr->flags ^= REMOVAL_FLAG_REMOVE;
    gp_debug("    Changed banksel (addr:%#lx state:%#lx action:%s)",
		Ptr->address, Ptr->state, Ptr->flags & REMOVAL_FLAG_REMOVE ? "remove" : "keep");
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_GOTO) {
    /* goto instructions and the like
     * exit state is the exit state of the previous instruction
     */
    uint32_t entry = Ptr->prev->state;
    if (entry == Ptr->state) {
      return false;
    }

    Ptr->state = entry;
    gp_debug("    Changed reloc (addr:%#lx state:%#lx)", Ptr->address, Ptr->state);
    return true;
  }

  if (Ptr->type == REMOVAL_TYPE_LABEL) {
    /* labels linked from other instructions
     * exit state is the combination of the exit state of the previous instruction
     * and the exit states of the previous instructions of all the instructions linked to it
     */
    uint32_t    entry = Ptr->prev->state;
    _removal_t *src = Ptr->source;

    while (src != NULL) {
      entry |= src->prev->state;
      src = src->source;
    }

    if (entry == Ptr->state) {
      return false;
    }

    Ptr->state = entry;
    gp_debug("    Changed label (addr:%#lx state:%#lx)", Ptr->address, Ptr->state);
    return true;
  }

  /* any other type is not required */
  Ptr->flags |= REMOVAL_FLAG_FIXED;
  return false;
}

/*------------------------------------------------------------------------------------------------*/

/* Deletes the unnecessary Banksel directives from an object. */

void
gp_coffopt_remove_unnecessary_banksel_experimental(gp_object_t *Object)
{
  _removal_object = Object;
  _removal_class = Object->class;

  if ((_removal_class != PROC_CLASS_PIC12)   && (_removal_class != PROC_CLASS_PIC12E) &&
      (_removal_class != PROC_CLASS_PIC12I)  && (_removal_class != PROC_CLASS_SX)     &&
      (_removal_class != PROC_CLASS_PIC14)   && (_removal_class != PROC_CLASS_PIC14E) &&
      (_removal_class != PROC_CLASS_PIC14EX) && (_removal_class != PROC_CLASS_PIC16)  &&
      (_removal_class != PROC_CLASS_PIC16E)) {
    return;
  }

  _removal_processor = Object->processor;
  _banksel_banks = gp_processor_num_banks(_removal_processor);

  gp_debug("Removing unnecessary banksel instructions (class:%s processor:%s banks:%u)",
           gp_processor_class_to_str(_removal_class), gp_processor_name(_removal_processor,0), _banksel_banks);
  assert(_banksel_banks <= BANKSEL_MAX_BANKS);

  /* Build the list of all the relocation items related to banksel. */
  /* Return true if there is something to remove. */
  if (!_removal_prepare_list(&_banksel_add_reloc)) {
    gp_debug("  There are no banksel instructions to remove.");
    return;
  }

  /* Mark all the symbols that can be changed by the banksel removal optimization code. */
  _removal_mark_symbols();

  /* Identify the banksel instructions that can be removed. */
  /* Return true if there are banksel instructions to remove. */
  if (!_removal_process_list(&_banksel_check_state)) {
    gp_debug("  There are no banksel instructions that can be removed.");
    return;
  }

  /* Remove the unnecessary instructions. */
  _removal_remove_instructions();

  /* Free the relocations list. */
  _removal_free_list();

  /* Rebuild the cinit table. */
  gp_cofflink_add_cinit_section(Object);
}

/*------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------
 *
 * Exclusive code for experimental pcallw removal.
 *
 *----------------------------------------------------------------*/

/* Return true if the symbol has a PCALLW stub. */
static gp_boolean
_is_MOVFW(gp_section_t *Section, uint32_t Byte_addr)
{
  uint16_t      data;
  const insn_t *insn;

  if (_removal_class->find_insn == NULL) {
    return false;
  }

  if (_removal_class->i_memory_get(Section->data, Byte_addr, &data, NULL, NULL) != W_USED_ALL) {
    gp_error("No instruction at 0x%0*X in program memory!", _removal_class->addr_digits, Byte_addr);
    assert(0);
    return false;
  }

  /* The instruction must be MOVF WCALLP,W. */
  insn = _removal_class->find_insn(_removal_class, data);
  if ((insn == NULL) || (insn->icode != ICODE_MOVF)) {
    return false;
  }

  gp_debug ("    Found instruction %s mask=%u opcode=%u icode=%u class=%u imask=%u attr=%u data=%#hx",
    insn->name, insn->mask, insn->opcode, insn->icode, insn->class, insn->inv_mask, insn->attribs, data);

  return true;
}

/* Append to the list the relocation item if it is related to pcallw removal. */
/* Return true if removal must be done. */
static gp_boolean
_pcallw_add_reloc(gp_section_t *Section, gp_reloc_t *Reloc)
{
  _removal_t   *ptr;
  uint16_t      type = 0;
  gp_symbol_t  *symbol = Reloc->symbol;
  uint32_t      addr = Section->address + Reloc->address;

  /* The relocation uses the PCALLW symbol. */
  if (!strcmp("PCALLW", symbol->name)) {
    if (_is_MOVFW(Section, addr)) {
      /* And points to a MOVFW instruction. */
      type = REMOVAL_TYPE_PCALLW_STUB;
      gp_debug("    Adding STUB to reloc:%hu section:%s addr:%#lx",
               Reloc->type, Section->name, addr);
    }
    else {
      /* But doesn't point to a MOVFW instruction. */
      type = REMOVAL_TYPE_PCALLW_INSN;
      gp_debug("    Adding INSN to reloc:%hu section:%s addr:%#lx",
               Reloc->type, Section->name, addr);
    }
  }
  else {
    switch (Reloc->type) {
      case RELOC_LOW:
      case RELOC_HIGH:
      case RELOC_UPPER:
        /* Ignore symbols without section (cinit). */
        if (symbol->section) {
          /* A relocation taking the symbol's address, maybe to build a pointer to the function. */
          type = REMOVAL_TYPE_PCALLW_ADDR;
          gp_debug("    Adding ADDR to reloc:%hu section:%s addr:%#lx symbol %s (class:%u value:%#lx section:%s flags:%#lx)",
                   Reloc->type, Section->name, addr, symbol->name, symbol->class, symbol->value, symbol->section->name, symbol->section->flags);
          break;
	}
	else {
          gp_debug("    Ignoring ADDR to reloc:%hu section:%s addr:%#lx symbol %s (class:%u value:%#lx no section)",
                   Reloc->type, Section->name, addr, symbol->name, symbol->class, symbol->value);
	}

      default:
        return false;
    }
  }

  /* add the relocation to the list */
  ptr = _removal_add_reloc (type, 2, 0, Section, Reloc);
  /* add a label for the relocation */
  if (type == REMOVAL_TYPE_PCALLW_ADDR) {
    _removal_add_label (Reloc->symbol, ptr);
  }

  return (type == REMOVAL_TYPE_PCALLW_STUB) ? true : false;
}

/* Check the state of a relocation element. */
/* Return true if it has changed, false otherwise. */
static gp_boolean
_pcallw_check_state(_removal_t *Ptr)
{
  _removal_t *next;

  if (Ptr->type == REMOVAL_TYPE_PCALLW_STUB) {
    next = Ptr->next;
    while ((next != NULL) && (next->type != REMOVAL_TYPE_LABEL) && (next->address <= (Ptr->address + 1))) {
      next = next->next;
    }

    if ((next != NULL) && (next->type == REMOVAL_TYPE_LABEL) && (next->address == (Ptr->address + 1))) {
      /* PCALLW stubs must be kept if the following label is used. */
      gp_debug("    Keeping stub (section:%s addr:%#lx)", Ptr->section->name, Ptr->address);
    }
    else {
      /* Otherwise can be removed. */
      Ptr->flags |= REMOVAL_FLAG_REMOVE;
      gp_debug("    Removing stub (section:%s addr:%#lx)", Ptr->section->name, Ptr->address);
    }
  }

  /* no more changes */
  Ptr->flags |= REMOVAL_FLAG_FIXED;
  return false;
}

/*------------------------------------------------------------------------------------------------*/

/* Removes the unused PCALLW stubs from an object. */

void
gp_coffopt_remove_unnecessary_pcallw_experimental(gp_object_t *Object)
{
  _removal_object = Object;
  _removal_class = Object->class;

  if ((_removal_class != PROC_CLASS_PIC12)   && (_removal_class != PROC_CLASS_PIC12E) &&
      (_removal_class != PROC_CLASS_PIC12I)  && (_removal_class != PROC_CLASS_SX)     &&
      (_removal_class != PROC_CLASS_PIC14)   && (_removal_class != PROC_CLASS_PIC14E) &&
      (_removal_class != PROC_CLASS_PIC14EX)) {
    return;
  }

  _removal_processor = Object->processor;

  gp_debug("Removing unused PCALLW stubs (class:%s processor:%s)",
           gp_processor_class_to_str(_removal_class), gp_processor_name(_removal_processor,0));

  /* Build the list of all the relocation items related to pcallw */
  /* Return true if there is something to remove */
  if (!_removal_prepare_list(&_pcallw_add_reloc)) {
    gp_debug ("  There are no stubs to remove.");
    return;
  }

  /* Mark all the symbols that can be changed by the pcallw removal code */
  _removal_mark_symbols();

  /* Identify the pcallw stubs that can be removed */
  /* Return true if there are stubs to remove */
  if (!_removal_process_list(&_pcallw_check_state)) {
    gp_debug ("  There are no stubs to remove.");
    return;
  }

  /* Remove the unnecessary instructions */
  _removal_remove_instructions();

  /* Free the relocations list */
  _removal_free_list();

  /* Rebuild the cinit table */
  gp_cofflink_add_cinit_section(Object);
}
