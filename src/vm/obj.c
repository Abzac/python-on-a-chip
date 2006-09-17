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
#define __FILE_ID__ 0x0F
/**
 * Object Type
 *
 * Object type operations.
 *
 * Log
 * ---
 *
 * 2006/08/31   #9: Fix BINARY_SUBSCR for case stringobj[intobj]
 * 2006/08/29   #15 - All mem_*() funcs and pointers in the vm should use
 *              unsigned not signed or void
 * 2002/05/04   First.
 */

/***************************************************************
 * Includes
 **************************************************************/

#include "py.h"


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

PyReturn_t
obj_loadFromImg(PyMemSpace_t memspace, P_U8 *paddr, pPyObj_t * r_pobj)
{
    PyReturn_t retval = PY_RET_OK;
    PyObjDesc_t od;

    /* get the object descriptor */
    od.od_type = (PyType_t)mem_getByte(memspace, paddr);

    switch (od.od_type)
    {
        /* if it's the None object, return global None */
        case OBJ_TYPE_NON:
            /* make sure *paddr gets incremented */
            *r_pobj = PY_NONE;
            break;

        /* if it's a simple type */
        case OBJ_TYPE_INT:
        case OBJ_TYPE_FLT:
            /* allocate simple obj */
            retval = heap_getChunk(sizeof(PyInt_t), (P_U8 *)r_pobj);
            PY_RETURN_IF_ERROR(retval);

            /* Set the object's type */
            (*r_pobj)->od.od_type = od.od_type;

            /* Read in the object's value (little endian) */
            ((pPyInt_t)*r_pobj)->val = mem_getInt(memspace, paddr);
            break;

        case OBJ_TYPE_STR:
            retval = string_loadFromImg(memspace, paddr, r_pobj);
            break;

        case OBJ_TYPE_TUP:
            retval = tuple_loadFromImg(memspace, paddr, r_pobj);
            break;

        case OBJ_TYPE_LST:
        case OBJ_TYPE_DIC:
        case OBJ_TYPE_COB:
        case OBJ_TYPE_MOD:
        case OBJ_TYPE_CLO:
        case OBJ_TYPE_FXN:
        case OBJ_TYPE_CLI:
            /* these types should not be in an img obj */
            return PY_RET_EX_SYS;

        /* if it's a native code img, load into a code obj */
        case OBJ_TYPE_NIM:
            retval = no_loadFromImg(memspace, paddr, r_pobj);
            break;

        /* if it's a code img, load into a code obj */
        case OBJ_TYPE_CIM:
            retval = co_loadFromImg(memspace, paddr, r_pobj);
            break;

        default:
            /* XXX invalid type for image obj */
            PY_ERR(__LINE__);
    }
    return retval;
}


S8
obj_isType(pPyObj_t pobj, PyType_t type)
{
    /* if null pointer or wrong type... */
    if ((pobj == C_NULL) || (pobj->od.od_type != type))
    {
        return C_FALSE;
    }
    return C_TRUE;
}

/* return true if the obj is false */
S8
obj_isFalse(pPyObj_t pobj)
{
    /* return true if it's NULL or None */
    if ((pobj == C_NULL) ||
        (pobj->od.od_type == OBJ_TYPE_NON))
    {
        return C_TRUE;
    }

    /* the integer zero is false */
    if ((pobj->od.od_type == OBJ_TYPE_INT) &&
        (((pPyInt_t)pobj)->val == 0))
    {
        return C_TRUE;
    }

    /* the floating point value of 0.0 is false */
    /*
    if ((pobj->od.od_type == OBJ_TYPE_FLT) &&
        (((pPyFloat)pobj)->val == 0.0))
    {
        retrun C_TRUE;
    }
    */

    /* an empty string is false */
    if (pobj->od.od_type == OBJ_TYPE_STR)
    {
        /* XXX this is for null-term string */
        return ((pPyString_t)pobj)->val[0] == C_NULL;
    }

    /* XXX the following are waiting on length fields */
    /* an empty tuple is false */
    /* an empty list is false */
    /* an empty dict is false */

    /*
     * the following types are always not false:
     * CodeObj, Function, Module, Class, ClassInstance.
     */
    return C_FALSE;
    /* XXX what about NATive and CIM code img? */
}


S8
obj_compare(pPyObj_t pobj1, pPyObj_t pobj2)
{
    /* null pointers are invalid */
    if ((pobj1 == C_NULL) || (pobj2 == C_NULL))
    {
        return C_DIFFER;
    }

    /* check if pointers are same */
    if (pobj1 == pobj2)
    {
        return C_SAME;
    }

    /* if types are different, return false */
    if (pobj1->od.od_type != pobj2->od.od_type)
    {
        return C_DIFFER;
    }

    /* else handle types individually */
    switch (pobj1->od.od_type)
    {
        case OBJ_TYPE_NON:
            return C_SAME;

        case OBJ_TYPE_INT:
        case OBJ_TYPE_FLT:
            return ((pPyInt_t)pobj1)->val ==
                   ((pPyInt_t)pobj2)->val ? C_SAME : C_DIFFER;

        case OBJ_TYPE_STR:
            return string_compare((pPyString_t)pobj1, (pPyString_t)pobj2);

        case OBJ_TYPE_TUP:
        case OBJ_TYPE_LST:
        case OBJ_TYPE_DIC:
        default:
            /* XXX fix these */
            return C_DIFFER;
    }

    /* XXX fix these */
    /* all other types would need same pointer to be true */
    return C_DIFFER;
}


/***************************************************************
 * Test
 **************************************************************/

/***************************************************************
 * Main
 **************************************************************/
