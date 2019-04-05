/*
 * LinearTransferFunctionParam.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#ifndef MEGAMOLCORE_LINEARTRANSFERFUNCTIONPARAM_H_INCLUDED
#define MEGAMOLCORE_LINEARTRANSFERFUNCTIONPARAM_H_INCLUDED


#include <string>
#include <cmath>

#include "mmcore/api/MegaMolCore.std.h"
#include "AbstractParam.h"

#include "vislib/String.h"
#include "vislib/sys/Log.h"

#include "json.hpp"


namespace megamol {
namespace core {
namespace param {


/**
 * Class for parameter holding transfer function as JSON string.
 */
class MEGAMOLCORE_API LinearTransferFunctionParam : public AbstractParam {
public:

    /** Interpolstion modes. */
    enum InterpolationMode { 
        LINEAR = 0, 
        GAUSS = 1 
    };

    /** Data type for transfer function data. */
    typedef std::vector<std::array<float, 5>> TFDataType;

    // ------------------------------------------------------------------------

    // Example JSON output format (empty string is also excepted for initialisation):
    //{
    //    "Interpolation": "LINEAR",
    //    "Nodes" : [
    //        [
    //            0.0, // = Red
    //            0.0, // = Green
    //            0.0, // = Blue
    //            0.0, // = Alpha
    //            0.0  // = Value
    //        ],
    //        [
    //            1.0, // = Red
    //            1.0, // = Green
    //            1.0, // = Blue
    //            1.0, // = Alpha
    //            1.0  // = Value
    //        ]
    //    ],
    //    "TextureSize": 128
    //}

    /**
    * Create transfer function texture from JSON string.
    *
    * @param in_tfs            The transfer function input encoded as string in JSON format.
    * @param out_data          The transfer function texture data output.
    * @param out_texsize       The transfer function texture size output.
    *
    * @return True if JSON string was successfully converted into transfer function texture, false otherwise.
    */
    static bool TransferFunctionTexture(const std::string &in_tfs, std::vector<float> &out_data, UINT &out_texsize);

    /**
    * Set transfer function data from JSON string.
    *
    * @param in_tfs            The transfer function input encoded as string in JSON format.
    * @param out_data          The transfer function data output.
    * @param out_interpolmode  The transfer function interpolation mode output.
    * @param out_texsize       The transfer function texture size output.
    *
    * @return True if JSON string was successfully converted into transfer function data, false otherwise.
    */
    static bool ParseTransferFunction(const std::string &in_tfs, TFDataType &out_data, InterpolationMode &out_interpolmode, UINT &out_texsize);

    /**
     * Get transfer function JSON string from data.
     *
     * @param out_tfs          The transfer function output encoded as string in JSON format.
     * @param in_data          The transfer function data input.
     * @param in_interpolmode  The transfer function interpolation mode input.
     * @param in_texsize       The transfer function texture size input.
     *
     * @return True if transfer function data was successfully converted into JSON string, false otherwise.
     */
    static bool DumpTransferFunction(std::string &out_tfs, const TFDataType &in_data, const InterpolationMode in_interpolmode, const UINT in_texsize);

    /**
     * Check given transfer function data.
     *
     * @param data          The transfer function data input.
     * @param interpolmode  The transfer function interpolation mode input.
     * @param texsize       The transfer function texture size input.
     *
     * @return True if given data is valid, false otherwise.
     */
    static bool CheckTransferFunctionData(const TFDataType &data, const InterpolationMode interpolmode, const UINT texsize);

    /**
     * Check given transfer function JSON string.
     *
     * @param tfs  The transfer function JSON string.
     *
     * @return True if given data is valid, false otherwise.
     */
    static bool CheckTransferFunctionString(const std::string &tfs);

    /**
    * Linear interpolation of transfer function data in range [0..texsize]
    *
    * @param out_texdata  The generated texture from the input transfer function.
    * @param in_texsize   The transfer function texture size input.
    * @param in_tfdata    The transfer function data input.
    */
    static void LinearInterpolation(std::vector<float> &out_texdata, unsigned int in_texsize, const TFDataType &in_tfdata);

    /**
    * Gauss interpolation of transfer function data in range [0..texsize]
    *
    * @param out_texdata  The generated texture from the input transfer function.
    * @param in_texsize   The transfer function texture size input.
    * @param in_tfdata    The transfer function data input.
    */
    static void GaussInterpolation(std::vector<float> &out_texdata, unsigned int in_texsize, const TFDataType &in_tfdata);

    /** Calculates gauss function for given parameters. */
    static float gauss(float x, float a, float b, float c) {
        //return ((1.0 / (s * sqrt(2.0*3.14159265358979323846))) * exp(-0.5 * ((x - u) / s) * ((x - u) / s)));
        return (a * expf(-1.0f * (x - b) * (x - b) / (2.0 * c * c)));
    }

    // ------------------------------------------------------------------------

    /**
     * Ctor.
     *
     * @param initVal The initial value
     */
    LinearTransferFunctionParam(const std::string& initVal = "");

    /**
     * Ctor.
     *
     * @param initVal The initial value
     */
    LinearTransferFunctionParam(const char *initVal);

    /**
     * Ctor.
     *
     * @param initVal The initial value
     * @param visible If 'true' the parameter is visible in the gui.
     */
    LinearTransferFunctionParam(const vislib::StringA& initVal);

    /**
     * Dtor.
     */
    virtual ~LinearTransferFunctionParam(void);

    /**
     * Returns a machine-readable definition of the parameter.
     *
     * @param outDef A memory block to receive a machine-readable
     *               definition of the parameter.
     */
    virtual void Definition(vislib::RawStorage& outDef) const override;

    /**
     * Tries to parse the given string as value for this parameter and
     * sets the new value if successful. This also triggers the update
     * mechanism of the slot this parameter is assigned to.
     *
     * @param v The new value for the parameter as string.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    virtual bool ParseValue(const vislib::TString& v) override;

    /**
    * Sets the value of the parameter and optionally sets the dirty flag
    * of the owning parameter slot.
    *
    * @param v the new value for the parameter
    * @param setDirty If 'true' the dirty flag of the owning parameter
    *                 slot is set and the update callback might be called.
    */
    void SetValue(const std::string& v, bool setDirty = true);

    /**
    * Returns the value of the parameter as string.
    *
    * @return The value of the parameter as string.
    */
    virtual vislib::TString ValueString(void) const override;

    /**
    * Gets the value of the parameter
    *
    * @return The value of the parameter
    */
    inline const std::string& Value(void) const {
        return this->val;
    }

    /**
     * Gets the value of the parameter
     *
     * @return The value of the parameter
     */
    inline operator const std::string&(void) const {
        return this->val;
    }

private:

#ifdef _WIN32
#pragma warning (disable: 4251)
#endif /* _WIN32 */

    /** The value of the parameter */
    std::string val;

#ifdef _WIN32
#pragma warning (disable: 4251)
#endif /* _WIN32 */

}; /* end class LinearTransferFunctionParam */

} /* end namespace param */
} /* end namespace core */
} /* end namespace megamol */

#endif /* end ifndef MEGAMOLCORE_LINEARTRANSFERFUNCTIONPARAM_H_INCLUDED */