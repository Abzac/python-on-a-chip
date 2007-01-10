/*
 * PyMite - A flyweight Python interpreter for 8-bit microcontrollers and more.
 * Copyright 2002 Dean Hall
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#undef __FILE_ID__
#define __FILE_ID__ 0x08
/**
 * Integer Object Type
 *
 * Integer object type operations.
 *
 * Log
 * ---
 *
 * 2007/01/09   Printing support (P.Adelt)
 * 2006/08/29   #15 - All mem_*() funcs and pointers in the vm should use
 *              unsigned not signed or void
 * 2002/05/04   First.
 */

/***************************************************************
 * Includes
 **************************************************************/

#include "pm.h"


/***************************************************************
 * Constants
 **************************************************************/

/***************************************************************
 * Macros
 **************************************************************/

/***************************************************************
 * Types
 **************************************************************/

/***************************************************************
 * Globals
 **************************************************************/

/***************************************************************
 * Prototypes
 **************************************************************/

/***************************************************************
 * Functions
 **************************************************************/

PmReturn_t
int_dup(pPmObj_t pint, pPmObj_t * r_pint)
{
    PmReturn_t retval = PM_RET_OK;

    /* allocate new int */
    retval = heap_getChunk(sizeof(PmInt_t), (uint8_t **)r_pint);
    PM_RETURN_IF_ERROR(retval);

    /* copy value */
    OBJ_SET_TYPE(**r_pint, OBJ_TYPE_INT);
    ((pPmInt_t)*r_pint)->val = ((pPmInt_t)pint)->val;
    return retval;
}


PmReturn_t
int_new(int32_t n, pPmObj_t *r_pint)
{
    PmReturn_t retval = PM_RET_OK;

    /* if n is 0,1,-1, return global int */
    if (n == 0)
    {
        *r_pint = PM_ZERO;
        return PM_RET_OK;
    }
    if (n == 1)
    {
        *r_pint = PM_ONE;
        return PM_RET_OK;
    }
    if (n == -1)
    {
        *r_pint = PM_NEGONE;
        return PM_RET_OK;
    }

    /* XXX search for int in pool? */

    /* else create and return new int obj */
    retval = heap_getChunk(sizeof(PmInt_t), (uint8_t **)r_pint);
    PM_RETURN_IF_ERROR(retval);
    OBJ_SET_TYPE(**r_pint, OBJ_TYPE_INT);
    ((pPmInt_t)*r_pint)->val = n;
    return retval;
}


PmReturn_t
int_positive(pPmObj_t pobj, pPmObj_t * r_pint)
{
    PmReturn_t retval;

    /* ensure it's an int */
    if (OBJ_GET_TYPE(*pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* create new int obj */
    return int_new(((pPmInt_t)pobj)->val, r_pint);
}


PmReturn_t
int_negative(pPmObj_t pobj, pPmObj_t * r_pint)
{
    PmReturn_t retval;

    /* ensure it's an int */
    if (OBJ_GET_TYPE(*pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* create new int obj */
    return int_new(-((pPmInt_t)pobj)->val, r_pint);
}


PmReturn_t
int_bitInvert(pPmObj_t pobj, pPmObj_t * r_pint)
{
    PmReturn_t retval;

    /* ensure it's an int */
    if (OBJ_GET_TYPE(*pobj) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

    /* create new int obj */
    return int_new(~((pPmInt_t)pobj)->val, r_pint);
}

#ifdef HAVE_PRINT
PmReturn_t
int_print(pPmObj_t pint)
{
	/* 2^31-1 has 10 decimal digits, plus sign and zero byte */
	uint8_t tBuffer[10+1+1];
	uint8_t bytesWritten, k;
    PmReturn_t retval = PM_RET_OK;

	C_ASSERT(pint != C_NULL);

    /* ensure string obj */
    if (OBJ_GET_TYPE(*pint) != OBJ_TYPE_INT)
    {
        PM_RAISE(retval, PM_RET_EX_TYPE);
        return retval;
    }

	#ifdef TARGET_AVR
	bytesWritten = snprintf_P((uint8_t*)&tBuffer, sizeof(tBuffer),
		PSTR("%li"), pint->val);
	#else
	/* This does not use snprintf because glibc's snprintf is only
	 * included for compiles without strict-ansi.
	 */
	bytesWritten = sprintf((uint8_t*)&tBuffer, "%li", ((pPmInt_t)pint)->val);
	#endif /* !TARGET_AVR */
	
		
	/* Sanity check */
	C_ASSERT(bytesWritten != 0);
	C_ASSERT(bytesWritten < sizeof(tBuffer));
		
	for (k=0; k<bytesWritten; k++)
	{
		retval = plat_putByte(tBuffer[k]);
    	PM_RETURN_IF_ERROR(retval);
	}
	
    return PM_RET_OK;
}
#endif /* HAVE_PRINT */

/***************************************************************
 * Test
 **************************************************************/

/***************************************************************
 * Main
 **************************************************************/
