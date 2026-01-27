/* Write coff objects
   Copyright (C) 2003, 2004, 2005
   Craig Franklin

   Copyright (C) 2017 Molnár Károly

   Dump COFF file contents option.
   Copyright (C) 2019 Gonzalo Pérez de Olaguer Córdoba <salo@gpoc.es>

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

#ifdef GPUTILS_DEBUG
gp_boolean gp_dump_coff = false;
#endif

/* String table offsets are 16 bits so this coff has a limit on the maximum string table size. */
#define MAX_STRING_TABLE      0xffff

static uint8_t      string_table[MAX_STRING_TABLE];

static uint32_t     string_offsets[MAX_STRING_TABLE + 1];
static unsigned int string_offs_num;
static uint32_t     last_string_size;

/*------------------------------------------------------------------------------------------------*/

static uint32_t
_search_string(const uint8_t* Table, const uint8_t* String, uint32_t String_size)
{
  uint32_t     offs;
  uint32_t     size;
  unsigned int i;
  unsigned int e;

  if (string_offs_num == 0) {
    return UINT32_MAX;
  }

  if (string_offs_num == 1) {
    if (last_string_size != String_size) {
      return UINT32_MAX;
    }

    offs = string_offsets[0];
    return (memcmp(&Table[offs], String, String_size) == 0) ? offs : UINT32_MAX;
  }

  e = string_offs_num - 1;

  for (i = 0; i < e; ++i) {
    offs = string_offsets[i];
    size = string_offsets[i + 1] - offs;
    if ((size == String_size) && (memcmp(&Table[offs], String, String_size) == 0)) {
      return offs;
    }
  }

  if (last_string_size != String_size) {
    return UINT32_MAX;
  }

  offs = string_offsets[i];
  return (memcmp(&Table[offs], String, String_size) == 0) ? offs : UINT32_MAX;
}

/*------------------------------------------------------------------------------------------------*/

static void
_store_string(uint8_t* Table, uint32_t Offset, const uint8_t* String, uint32_t String_size)
{
  memcpy(&Table[Offset], String, String_size);
  string_offsets[string_offs_num] = Offset;
  ++string_offs_num;
  last_string_size = String_size;
}

/*------------------------------------------------------------------------------------------------*/

/* Write the symbol or section name into the string table. */

static unsigned int
_add_string(const char* String, uint8_t* Table)
{
  uint32_t nbytes;
  uint32_t offset;
  uint32_t string_size;

  assert(!(String == NULL));

  string_size = (uint32_t)(strlen(String) + 1);
  offset      = _search_string(Table, (const uint8_t*)String, string_size);
  if (offset == UINT32_MAX) {
    /* Read the number of bytes in the string table. */
    offset = nbytes = gp_getu32(Table);

    /* Check the length against the max string table size. */
    nbytes += string_size;
    assert(!(nbytes > MAX_STRING_TABLE));

    /* Copy the string to the table. */
    _store_string(Table, offset, (const uint8_t*)String, string_size);

    /* Write the new byte count. */
    gp_putl32(Table, nbytes);
  }

  return offset;
}

/*------------------------------------------------------------------------------------------------*/

static void
_add_name(const char *Name, uint8_t *Table, FILE *Fp)
{
  uint32_t length;
  uint32_t offset;

  if (Name == NULL) {
    return;
  }

  length = (uint32_t)strlen(Name);

  if (length <= COFF_SSYMBOL_NAME_MAX) {
    /* The string will fit in the structure. */
    if (length > 0) {
      /* 'name' */
      gp_fputvar(Name, length, Fp);     /* symbol name if less then 8 characters */
    }

    if (length < COFF_SSYMBOL_NAME_MAX) {
      gp_fputzero(COFF_SSYMBOL_NAME_MAX - length, Fp);
    }
  }
  else {
    offset = _add_string(Name, Table);

    /* write zeros and offset */
    /* 's_zeros' */
    gp_fputl32(0, Fp);                  /* first four characters are 0 */
    /* 's_offset' */
    gp_fputl32(offset, Fp);             /* pointer to the string table */
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the file header */

static void
_write_file_header(const gp_object_t *Object, FILE *Fp)
{
  /* 'f_magic'  -- magic number */
  gp_fputl16((Object->isnew ? MICROCHIP_MAGIC_v2 : MICROCHIP_MAGIC_v1), Fp);
  /* 'f_nscns'  -- number of sections */
  gp_fputl16((uint16_t)Object->section_list.num_nodes, Fp);
  /* 'f_timdat' -- time and date stamp */
  gp_fputl32(Object->time, Fp);
  /* 'f_symptr' -- file ptr to symtab */
  gp_fputl32(Object->symbol_ptr, Fp);
  /* 'f_nsyms'  -- # symtab entries */
  gp_fputl32(Object->num_symbols, Fp);
  /* 'f_opthdr' -- sizeof(opt hdr) */
  gp_fputl16((Object->isnew ? OPT_HDR_SIZ_v2: OPT_HDR_SIZ_v1), Fp);
  /* 'f_flags'  -- flags */
  gp_fputl16(Object->flags, Fp);
}

/*------------------------------------------------------------------------------------------------*/

/* write the optional header */

static void
_write_optional_header(const gp_object_t *Object, FILE *Fp)
{
  uint32_t coff_type;

  coff_type = gp_processor_coff_type(Object->processor);
  /* make sure we catch unfinished processors */
  assert(coff_type != 0);

  /* write the data to file */
  /* 'opt_magic' */
  gp_fputl16(Object->isnew ? OPTMAGIC_v2 : OPTMAGIC_v1, Fp);

  /* 'vstamp' -- version stamp of compiler */
  if (Object->isnew) {
    gp_fputl32(1, Fp);
  }
  else {
    gp_fputl16(1, Fp);
  }

  /* 'proc_type'      -- processor type */
  gp_fputl32(coff_type, Fp);
  /* 'rom_width_bits' -- ROM width bits */
  gp_fputl32(gp_processor_rom_width(Object->class), Fp);
  /* 'ram_width_bits' -- RAM width bits */
  gp_fputl32(8, Fp);
}

/*------------------------------------------------------------------------------------------------*/

/* write the section header */

static void
_write_section_header(const gp_section_t *Section, unsigned int Org_to_byte_shift, uint8_t *Table, FILE *Fp)
{
  uint32_t section_address;

  if (FlagsIsAllClr(Section->flags, STYP_ROM_AREA)) {
    Org_to_byte_shift = 0;
  }

  section_address = (uint32_t)gp_insn_from_byte(Org_to_byte_shift, (int)Section->address);
  _add_name(Section->name, Table, Fp);
  /* 's_paddr'   -- physical address */
  gp_fputl32(section_address, Fp);
  /* 's_vaddr'   -- virtual address */
  gp_fputl32(section_address, Fp);
  /* 's_size'    -- section size */
  gp_fputl32(Section->size, Fp);
  /* 's_scnptr'  -- file ptr to raw data */
  gp_fputl32(Section->data_ptr, Fp);
  /* 's_relptr'  -- file ptr to relocation */
  gp_fputl32(Section->reloc_ptr, Fp);
  /* 's_lnnoptr' -- file ptr to line numbers */
  gp_fputl32(Section->lineno_ptr, Fp);
  /* 's_nreloc'  -- # reloc entries */
  gp_fputl16((uint16_t)Section->relocation_list.num_nodes, Fp);
  /* 's_nlnno'   -- # line number entries */
  gp_fputl16((uint16_t)Section->line_number_list.num_nodes, Fp);
  /* 's_flags'   -- Don't write internal section flags. */
  gp_fputl32(Section->flags & (unsigned int)(~(STYP_RELOC | STYP_BPACK)), Fp);
}

/*------------------------------------------------------------------------------------------------*/

/* write the section data */

static void
_write_section_data(pic_processor_t Processor, const gp_section_t *Section, FILE *Fp)
{
  unsigned int org;
  unsigned int end;
  uint8_t      byte;

  org = Section->shadow_address;
  end = org + Section->size;

#ifdef GPUTILS_DEBUG
  if (gp_dump_coff) {
    printf("section \"%s\"\nsize= %li\ndata:\n", Section->name, Section->size);
    gp_mem_i_print(Section->data, Processor);
  }
#else
  (void)Processor;
#endif

  for ( ; org < end; org++) {
    gp_mem_b_assert_get(Section->data, org, &byte, NULL, NULL);
    fputc(byte, Fp);
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the section relocations */

static void
_write_relocations(const gp_section_t *Section, FILE *Fp)
{
  gp_reloc_t *current;

  current = Section->relocation_list.first;
  while (current != NULL) {
    /* 'r_vaddr'  -- entry relative virtual address */
    gp_fputl32(current->address, Fp);
    /* 'r_symndx' -- index into symbol table */
    gp_fputl32(current->symbol->number, Fp);
    /* 'r_offset' -- offset to be added to address of symbol 'r_symndx' */
    gp_fputl16(current->offset, Fp);
    /* 'r_type'   -- relocation type */
    gp_fputl16(current->type, Fp);

    current = current->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the section linenumbers */

static void
_write_linenumbers(const gp_section_t *Section, unsigned int Org_to_byte_shift, FILE *Fp)
{
  gp_linenum_t *current;

  if (FlagsIsAllClr(Section->flags, STYP_ROM_AREA)) {
    Org_to_byte_shift = 0;
  }

  current = Section->line_number_list.first;
  while (current != NULL) {
    /* 'l_srcndx' -- symbol table index of associated source file */
    gp_fputl32(current->symbol->number, Fp);
    /* 'l_lnno'   -- line number */
    gp_fputl16(current->line_number, Fp);
    /* 'l_paddr'  -- address of code for this lineno */
    gp_fputl32(gp_insn_from_byte(Org_to_byte_shift, (int)current->address), Fp);
    /* 'l_flags'  -- bit flags for the line number */
    gp_fputl16(0, Fp);
    /* 'l_fcnndx' -- symbol table index of associated function, if there is one */
    gp_fputl32(0, Fp);

    current = current->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the auxiliary symbols */

static void
_write_aux_symbols(const gp_aux_t *Aux, uint8_t *Table, gp_boolean Isnew, FILE *Fp)
{
  unsigned int offset;

  while (Aux != NULL) {
    switch (Aux->type) {
      case AUX_DIRECT: {
        /* add the direct string to the string table */
        offset = _add_string(Aux->_aux_symbol._aux_direct.string, Table);
        /* 'x_command' */
        gp_fputl32(Aux->_aux_symbol._aux_direct.command, Fp);
        /* 'x_offset' */
        gp_fputl32(offset, Fp);
        /* _unused */
        gp_fputzero(10, Fp);

        if (Isnew) {
          /* _unused */
          gp_fputzero(2, Fp);
        }
        break;
      }

      case AUX_FILE: {
        /* add the filename to the string table */
        offset = _add_string(Aux->_aux_symbol._aux_file.filename, Table);
        /* 'x_offset' */
        gp_fputl32(offset, Fp);
        /* 'x_incline' */
        gp_fputl32(Aux->_aux_symbol._aux_file.line_number, Fp);
        /* x_flags */
        fputc(Aux->_aux_symbol._aux_file.flags, Fp);
        /* _unused */
        gp_fputzero(9, Fp);

        if (Isnew) {
          /* _unused */
          gp_fputzero(2, Fp);
        }
        break;
      }

      case AUX_IDENT: {
        /* add the ident string to the string table */
        offset = _add_string(Aux->_aux_symbol._aux_ident.string, Table);
        /* 'x_offset' */
        gp_fputl32(offset, Fp);
        /* _unused */
        gp_fputzero(14, Fp);

        if (Isnew) {
          /* _unused */
          gp_fputzero(2, Fp);
        }
        break;
      }

      case AUX_SECTION: {
        /* write section auxiliary symbol */
        /* 'x_scnlen' */
        gp_fputl32(Aux->_aux_symbol._aux_scn.length, Fp);
        /* 'x_nreloc' */
        gp_fputl16(Aux->_aux_symbol._aux_scn.nreloc, Fp);
        /* 'x_nlinno' */
        gp_fputl16(Aux->_aux_symbol._aux_scn.nlineno, Fp);
        /* _unused */
        gp_fputzero(10, Fp);

        if (Isnew) {
          /* _unused */
          gp_fputzero(2, Fp);
        }
        break;
      }

      default:
        /* copy the data to the file */
        gp_fputvar(&Aux->_aux_symbol.data[0], ((Isnew) ? SYMBOL_SIZE_v2 : SYMBOL_SIZE_v1), Fp);
    }

    Aux = Aux->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the symbol table */

static void
_write_symbols(const gp_object_t *Object, uint8_t *Table, FILE *Fp)
{
  gp_symbol_t *symbol;
  gp_boolean   isnew;

  isnew   = Object->isnew;
  symbol = Object->symbol_list.first;
  while (symbol != NULL) {
    _add_name(symbol->name, Table, Fp);
    /* 'value' */
    gp_fputl32((int32_t)symbol->value, Fp);

    /* 'sec_num' */
    if (symbol->section_number < N_SCNUM) {
      gp_fputl16(symbol->section_number, Fp);
    }
    else {
      gp_fputl16((uint16_t)symbol->section->number, Fp);
    }

    /* 'type' */
    if (isnew) {
      gp_fputl32((uint32_t)symbol->type | (symbol->derived_type << T_SHIFT_v2), Fp);
    }
    else {
      gp_fputl16((uint16_t)symbol->type | (uint16_t)(symbol->derived_type << T_SHIFT_v1), Fp);
    }

    /* 'st_class' */
    fputc(symbol->class, Fp);
    /* 'num_auxsym' */
    fputc((int)symbol->aux_list.num_nodes, Fp);

    if (symbol->aux_list.num_nodes > 0) {
      _write_aux_symbols(symbol->aux_list.first, Table, isnew, Fp);
    }

    symbol = symbol->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* update all the coff pointers */

static void
_update_pointers(gp_object_t *Object)
{
  unsigned int  data_idx;
  gp_section_t *section;
  gp_symbol_t  *symbol;
  unsigned int  section_number;
  unsigned int  symbol_number;

  section_number = (unsigned int)Object->section_list.num_nodes;
  data_idx = (Object->isnew) ? (FILE_HDR_SIZ_v2 + OPT_HDR_SIZ_v2 + (SEC_HDR_SIZ_v2 * section_number)) :
                               (FILE_HDR_SIZ_v1 + OPT_HDR_SIZ_v1 + (SEC_HDR_SIZ_v1 * section_number));

  section_number = N_SCNUM;

  /* update the data pointers in the section headers */
  section = Object->section_list.first;
  while (section != NULL) {
    section->number = section_number;
    section_number++;
    section->data_ptr = 0;

    if (gp_coffgen_section_has_data(section)) {
      section->data_ptr = data_idx;
      data_idx         += section->size;
    }
    section = section->next;
  }

  /* update the relocation pointers in the section headers */
  section = Object->section_list.first;
  while (section != NULL) {
    section->reloc_ptr = 0;

    if (section->relocation_list.num_nodes > 0) {
      section->reloc_ptr = data_idx;
      data_idx          += (section->relocation_list.num_nodes * RELOC_SIZ);
    }
    section = section->next;
  }

  /* update the line number pointers in the section headers */
  section = Object->section_list.first;
  while (section != NULL) {
    section->lineno_ptr = 0;

    if (section->line_number_list.num_nodes > 0) {
      section->lineno_ptr = data_idx;
      data_idx           += (section->line_number_list.num_nodes * LINENO_SIZ);
    }
    section = section->next;
  }

  /* update symbol table pointer */
  Object->symbol_ptr = data_idx;

  /* update the symbol numbers */
  symbol_number = 0;
  symbol        = Object->symbol_list.first;
  while (symbol != NULL) {
    symbol->number = symbol_number;
    symbol_number += 1 + symbol->aux_list.num_nodes;
    symbol = symbol->next;
  }
}

/*------------------------------------------------------------------------------------------------*/

/* write the coff file */

gp_boolean
gp_writeobj_write_coff(gp_object_t *Object, int Num_errors)
{
  FILE         *coff;
  unsigned int  org_to_byte_shift;
  gp_section_t *section;

  if (Num_errors > 0) {
    unlink(Object->filename);
    return false;
  }

  coff = fopen(Object->filename, "wb");
  if (coff == NULL) {
    perror(Object->filename);
    return false;
  }

  string_offs_num  = 0;
  last_string_size = 0;

  /* update file pointers in the coff */
  _update_pointers(Object);

  /* initialize the string table byte count */
  gp_putl32(string_table, (uint32_t)sizeof(uint32_t));

  /* write the data to the file */
  _write_file_header(Object, coff);
  _write_optional_header(Object, coff);

  /* write section headers */
  org_to_byte_shift = Object->class->org_to_byte_shift;
  section           = Object->section_list.first;
  while (section != NULL) {
    _write_section_header(section, org_to_byte_shift, string_table, coff);
    section = section->next;
  }

  /* write section data */
  section = Object->section_list.first;
  while (section != NULL) {
    if (gp_coffgen_section_has_data(section)) {
      _write_section_data(Object->processor, section, coff);
    }
    section = section->next;
  }

  /* write section relocations */
  section = Object->section_list.first;
  while (section != NULL) {
    if (section->relocation_list.num_nodes > 0) {
      _write_relocations(section, coff);
    }
    section = section->next;
  }

  /* write section line numbers */
  section = Object->section_list.first;
  while (section != NULL) {
    if (section->line_number_list.num_nodes > 0) {
      _write_linenumbers(section, org_to_byte_shift, coff);
    }
    section = section->next;
  }

  /* write symbols */
  if (Object->num_symbols != 0) {
    _write_symbols(Object, string_table, coff);
  }

  /* write string table */
  fwrite(string_table, 1, gp_getl32(string_table), coff);

  fclose(coff);
  return true;
}
