#include "map_renderer.h"

namespace render{

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return render_settings_;
}

SphereProjector SphereProjector::GetSphereProjector(const std::vector<geo::Coordinates>& cords, const RenderSettings& renderer){
    return SphereProjector(cords.begin(), cords.end(), renderer.width, renderer.height, renderer.padding);
}
}
