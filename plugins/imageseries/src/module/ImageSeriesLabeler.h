/**
 * MegaMol
 * Copyright (c) 2022, MegaMol Dev Team
 * All rights reserved.
 */

#pragma once

#include "../filter/AsyncFilterRunner.h"
#include "../util/LRUCache.h"

#include "imageseries/AsyncImageData2D.h"
#include "imageseries/ImageSeries2DCall.h"

#include "mmcore/Call.h"
#include "mmcore/CalleeSlot.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/Module.h"
#include "mmcore/param/ParamSlot.h"

#include <memory>

namespace megamol::ImageSeries {

/**
 * Labels connected components in monochrome images within an image series.
 */
class ImageSeriesLabeler : public core::Module {

public:
    /**
     * @return The name of this module.
     */
    static const char* ClassName() {
        return "ImageSeriesLabeler";
    }

    /**
     * @return A human readable description of this module.
     */
    static const char* Description() {
        return "Labels connected components in monochrome images within an image series.";
    }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable() {
        return true;
    }

    ImageSeriesLabeler();

    ~ImageSeriesLabeler() override;

protected:
    /**
     * Initializes this loader instance.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    bool create() override;

    /**
     * Releases all resources used by this loader instance.
     */
    void release() override;

    /**
     * Implementation of the getData call.
     */
    bool getDataCallback(core::Call& caller);

    /**
     * Implementation of the getMetaData call.
     */
    bool getMetaDataCallback(core::Call& caller);

    /**
     * Callback for changes to any of the filtering parameters.
     */
    bool filterParametersChangedCallback(core::param::ParamSlot& param);

private:
    core::CalleeSlot getDataCallee;

    core::CallerSlot getInputCaller;
    core::CallerSlot getMaskCaller;

    core::param::ParamSlot maskFrameParam;
    core::param::ParamSlot maskPriorityParam;
    core::param::ParamSlot negateMaskParam;
    core::param::ParamSlot minBlobSizeParam;
    core::param::ParamSlot thresholdParam;
    core::param::ParamSlot negateThresholdParam;
    core::param::ParamSlot flowFrontParam;
    core::param::ParamSlot flowFrontOffsetParam;

    util::LRUCache<typename AsyncImageData2D<>::Hash, AsyncImageData2D<>> imageCache;

    std::unique_ptr<filter::AsyncFilterRunner<>> filterRunner;
};

} // namespace megamol::ImageSeries
