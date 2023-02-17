#pragma once

#include "imageseries/AsyncImageData2D.h"
#include "imageseries/ImageSeries2DCall.h"
#include "imageseries/graph/GraphData2D.h"
#include "imageseries/graph/GraphData2DCall.h"
#include "imageseries/util/ImageUtils.h"

#include <memory>

namespace megamol::ImageSeries::filter {

class FlowTimeLabelFilter {
public:
    using AsyncImagePtr = std::shared_ptr<const AsyncImageData2D<>>;
    using ImagePtr = std::shared_ptr<const typename AsyncImageData2D<>::BitmapImage>;

    struct Output {
        std::shared_ptr<const vislib::graphics::BitmapImage> image;
        std::shared_ptr<const graph::GraphData2D> graph;
    };

    using Label = std::uint16_t;

    // Special labels
    enum LabelType : Label {
        // Pixel belonging to the solid phase / unoccupied space
        LabelSolid = 0,

        // Minimal label for flow
        LabelMinimum = 1,

        // Maximal label available
        LabelMaximum = 65532,

        // Empty space
        LabelEmpty = 65533,

        // Label for flow fronts that are "invalid" (e.g., too small)
        LabelInvalid = 65534,

        // Initially unassigned pixel
        LabelUnassigned = 65535,
    };

    static constexpr int LabelCount = 65536;

    struct Input {
        // Timestamp map within which to search for connected blobs.
        AsyncImagePtr timeMap;

        // Select output image
        enum class image_t : int { simplified, invalid, original } outputImage;

        // Inflow area
        enum class inflow_t : int { left, bottom, right, top } inflowArea;
        int inflowMargin;

        // Minimum obstacle size, used as parameter for graph simplification
        int minObstacleSize;

        // Applied graph fixes/simplifications
        enum class fixes_t : int {
            nope = 0,
            isolated = 1,
            false_sources = 2,
            false_sinks = 4,
            unimportant_sinks = 8,
            resolve_diamonds = 16,
            combine_trivial = 32,
            combine_tiny = 64
        } fixes;
    };

    FlowTimeLabelFilter(Input input);
    std::shared_ptr<Output> operator()();

    ImageMetadata getMetadata() const;

private:
    Input input;
};

} // namespace megamol::ImageSeries::filter

namespace std {
template<>
struct hash<std::shared_ptr<const megamol::ImageSeries::graph::GraphData2D>> {
    std::size_t operator()(std::shared_ptr<const megamol::ImageSeries::graph::GraphData2D> graphData) const {
        return graphData
                   ? megamol::ImageSeries::util::computeHash(graphData->getNodes().size(), graphData->getEdges().size())
                         : 0;
    }
};

template<>
struct hash<std::shared_ptr<const megamol::ImageSeries::filter::FlowTimeLabelFilter::Output>> {
    std::size_t operator()(
        std::shared_ptr<const megamol::ImageSeries::filter::FlowTimeLabelFilter::Output> data) const {
        return data ? megamol::ImageSeries::util::combineHash(std::hash<decltype(data->image)>()(data->image),
                          std::hash<decltype(data->graph)>()(data->graph))
                    : 0;
    }
};
} // namespace std

static auto operator&(megamol::ImageSeries::filter::FlowTimeLabelFilter::Input::fixes_t lhs,
    megamol::ImageSeries::filter::FlowTimeLabelFilter::Input::fixes_t rhs) {
    return static_cast<int>(lhs) & static_cast<int>(rhs);
}

static auto operator|(megamol::ImageSeries::filter::FlowTimeLabelFilter::Input::fixes_t lhs,
    megamol::ImageSeries::filter::FlowTimeLabelFilter::Input::fixes_t rhs) {
    return static_cast<megamol::ImageSeries::filter::FlowTimeLabelFilter::Input::fixes_t>(
        static_cast<int>(lhs) | static_cast<int>(rhs));
}
