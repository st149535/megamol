/*
 * SplitMergeCall.h
 *
 * Author: Guido Reina
 * Copyright (C) 2012 by Universitaet Stuttgart (VISUS).
 * All rights reserved.
 */


#ifndef MEGAMOL_PROTEIN_RESIDUE_SELECTIONCALL_H_INCLUDED
#define MEGAMOL_PROTEIN_RESIDUE_SELECTIONCALL_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif /* (defined(_MSC_VER) && (_MSC_VER > 1000)) */

#include "Call.h"
#include "CallAutoDescription.h"
#include "vislib/Array.h"

#include "ResidueSelection.h"

namespace megamol {
namespace protein {

    /**
     * Base class for graph calls and data interfaces.
     *
     * Graphs based on coordinates can contain holes where the respective
     * getters return false for the abscissae. For categorical graphs this
     * seems useless as the abscissa is sparse anyway, but the interface
     * allows for that as well.
     */

    class ResidueSelectionCall : public megamol::core::Call {
    public:
        /**
         * Answer the name of the objects of this description.
         *
         * @return The name of the objects of this description.
         */
        static const char *ClassName(void) {
            return "ResidueSelectionCall";
        }

        /**
         * Gets a human readable description of the module.
         *
         * @return A human readable description of the module.
         */
        static const char *Description(void) {
            return "Call to get selection Residues";
        }

        /** Index of the 'GetData' function */
        static const unsigned int CallForGetSelection;

        static const unsigned int CallForSetSelection;

        /**
         * Answer the number of functions used for this call.
         *
         * @return The number of functions used for this call.
         */
        static unsigned int FunctionCount(void) {
            return 2;
        }

        /**
         * Answer the name of the function used for this call.
         *
         * @param idx The index of the function to return it's name.
         *
         * @return The name of the requested function.
         */
        static const char* FunctionName(unsigned int idx) {
            switch (idx) {
            case 0:
                return "getSelection";
            case 1:
                return "setSelection";
            }
            return "";
        }

		inline vislib::Array<ResidueSelection::Residue> *GetSelectionPointer(void) const {
            return this->selection;
        }

        inline void SetSelectionPointer(vislib::Array<ResidueSelection::Residue> *selection) {
            this->selection = selection;
        }

        ResidueSelectionCall(void);
        virtual ~ResidueSelectionCall(void);

    private:
        vislib::Array<ResidueSelection::Residue> *selection;
    };

    /** Description class typedef */
    typedef megamol::core::CallAutoDescription<ResidueSelectionCall> ResidueSelectionCallDescription;

} /* end namespace protein */
} /* end namespace megamol */

#endif /* MEGAMOL_PROTEIN_RESIDUE_SELECTIONCALL_H_INCLUDED */
