/**
* KeyframeManipulators.cpp
*
* Copyright (C) 2017 by VISUS (Universitaet Stuttgart).
* Alle Rechte vorbehalten.
*/

#include "stdafx.h"
#include "KeyframeManipulators.h"


using namespace megamol;
using namespace megamol::core;
using namespace megamol::cinematic;



KeyframeManipulators::KeyframeManipulators(void)
    : toggleVisibleGroupParam("manipulators::toggleVisibleGroup", "Toggle visibility of different manipulator groups.")
    , visibleGroupParam("manipulators::visibleGroup", "Select visible manipulator group.")
    , toggleOusideBboxParam("manipulators::showOutsideBBox", "Show manipulators always outside of model bounding box.")
    , paramSlots()
    , visibleGroup(VisibleGroup::SELECTED_KEYFRAME_AND_CTRLPOINT_POSITION)
    , toggleOusideBbox(false)
    , manipulators()
    , selectors()
    , state()
    , point_radius(10.0f)
    , line_width(2.0f)
    , line_length(50.0f) {

    this->paramSlots.clear();
    this->selectors.clear();
    this->manipulators.clear();

    this->toggleOusideBboxParam.SetParameter(new param::ButtonParam(core::view::Key::KEY_W, core::view::Modifier::CTRL));
    this->paramSlots.emplace_back(&this->toggleOusideBboxParam);

    this->toggleVisibleGroupParam.SetParameter(new param::ButtonParam(core::view::Key::KEY_Q, core::view::Modifier::CTRL));
    this->paramSlots.emplace_back(&this->toggleVisibleGroupParam);

    param::EnumParam* vmg = new param::EnumParam(this->visibleGroup);
    vmg->SetTypePair(VisibleGroup::SELECTED_KEYFRAME_AND_CTRLPOINT_POSITION, "Keyframe and Ctrl-Point Positions");
    vmg->SetTypePair(VisibleGroup::SELECTED_KEYFRAME_LOOKAT_AND_UP_VECTOR, "LookAt Vector and Up Vector");
    this->visibleGroupParam << vmg;
    this->paramSlots.emplace_back(&this->visibleGroupParam);

    Manipulator m;
    m.show = false;
    m.vector = glm::vec3(0.0f, 0.0f, 0.0f);

    m.variety = Manipulator::Variety::MANIPULATOR_FIRST_CTRLPOINT_POSITION;
    m.rigging = Manipulator::Rigging::X_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Y_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Z_DIRECTION;
    this->manipulators.push_back(m);

    m.variety = Manipulator::Variety::MANIPULATOR_LAST_CTRLPOINT_POSITION;
    m.rigging = Manipulator::Rigging::X_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Y_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Z_DIRECTION;
    this->manipulators.push_back(m);

    m.variety = Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION;
    m.rigging = Manipulator::Rigging::X_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Y_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Z_DIRECTION;
    this->manipulators.push_back(m);

    m.variety = Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION_USING_LOOKAT;
    m.rigging = Manipulator::Rigging::VECTOR_DIRECTION;
    this->manipulators.push_back(m);

    m.variety = Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_LOOKAT_VECTOR;
    m.rigging = Manipulator::Rigging::X_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Y_DIRECTION;
    this->manipulators.push_back(m);
    m.rigging = Manipulator::Rigging::Z_DIRECTION;
    this->manipulators.push_back(m);

    m.variety = Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_UP_VECTOR;
    m.rigging = Manipulator::Rigging::ROTATION;
    this->manipulators.push_back(m);
}


KeyframeManipulators::~KeyframeManipulators(void) {

}


void KeyframeManipulators::UpdateExtents(vislib::math::Cuboid<float>& inout_bbox) {

    // First store copy of unmodified model bounding box
    this->state.bbox = inout_bbox;
    // Grow bounding box of model to manipulator positions
    glm::vec3 pos; 
    for (unsigned int i = 0; i < this->manipulators.size(); i++) {
        pos = this->getManipulatorPosition(this->manipulators[i]);
        pos = pos + this->manipulators[i].vector * this->line_length;
        inout_bbox.GrowToPoint(G2P(pos));
    }
}


bool KeyframeManipulators::UpdateRendering(const std::shared_ptr<std::vector<Keyframe>> keyframes, Keyframe selected_keyframe, glm::vec3 first_ctrl_pos, glm::vec3 last_ctrl_pos,
    const camera_state_type& minimal_snapshot, glm::vec2 viewport_dim, glm::mat4 mvp) {

    // Update parameters
    if (this->visibleGroupParam.IsDirty()) {
        this->visibleGroup = static_cast<VisibleGroup>(this->visibleGroupParam.Param<core::param::EnumParam>()->Value());
        this->visibleGroupParam.ResetDirty();
    }

    if (this->toggleVisibleGroupParam.IsDirty()) {
        this->visibleGroup = static_cast<VisibleGroup>((this->visibleGroup + 1) % VisibleGroup::VISIBLEGROUP_COUNT);
        this->toggleVisibleGroupParam.ResetDirty();
    }

    if (this->toggleOusideBboxParam.IsDirty()) {
        this->toggleOusideBbox = !this->toggleOusideBbox;
        this->toggleOusideBboxParam.ResetDirty();
    }

    // Update current state
    this->state.viewport = viewport_dim;
    this->state.mvp = mvp;
    this->state.cam_min_snapshot = minimal_snapshot;
    this->state.selected_keyframe = selected_keyframe;
    this->state.first_ctrl_point = first_ctrl_pos;
    this->state.last_ctrl_point = last_ctrl_pos;
    this->state.selected_index = -1;

    // Update primitive dimensions (empirical)
    float length = this->state.bbox.LongestEdge();
    this->line_length = length * 0.3f;
    this->line_width = this->line_length / 30.0f;
    this->point_radius = this->line_length / 6.0f;

    // Update keyframe position selectors
    auto keyframe_count = keyframes->size();
    this->selectors.resize(keyframe_count);
    std::array<float, 3> pos;
    for (size_t i = 0; i < keyframe_count; ++i) {
        this->selectors[i].show = true;
        this->selectors[i].variety = Manipulator::Variety::SELECTOR_KEYFRAME_POSITION;
        this->selectors[i].rigging = Manipulator::Rigging::NONE;
        pos = keyframes->operator[](i).GetCameraState().position;
        this->selectors[i].vector = glm::vec3(pos[0], pos[1], pos[2]);
        if (keyframes->operator[](i) == this->state.selected_keyframe) {
            this->state.selected_index = i;
        }
    }

    // Update selected keyframe manipulators
    view::Camera_2 cam(this->state.selected_keyframe.GetCameraState());
    cam_type::snapshot_type snapshot;
    cam.take_snapshot(snapshot, thecam::snapshot_content::view_vector | thecam::snapshot_content::up_vector);
    for (auto &m : this->manipulators) {
        // Set visibility depending on active group
        m.show = false;
        switch (this->visibleGroup) {
            case(VisibleGroup::SELECTED_KEYFRAME_AND_CTRLPOINT_POSITION): {
                switch (m.variety) {
                    case(Manipulator::Variety::MANIPULATOR_FIRST_CTRLPOINT_POSITION): {
                        if (this->state.selected_index == 0) {
                            m.show = true;
                        }
                    } break;
                    case(Manipulator::Variety::MANIPULATOR_LAST_CTRLPOINT_POSITION): {
                        if (this->state.selected_index == (keyframe_count - 1)) {
                            m.show = true;
                        }
                    } break;
                    case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION): {
                        m.show = true;
                    } break;
                    default: break;
                }
            } break;
            case(VisibleGroup::SELECTED_KEYFRAME_LOOKAT_AND_UP_VECTOR): {
                switch (m.variety) {
                    case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_LOOKAT_VECTOR): 
                    case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION_USING_LOOKAT): 
                    case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_UP_VECTOR): {
                        m.show = true;
                    } break;
                    default: break;
                }
            } break;
            default: break;
        }
        // Set direction depending on rigging
        switch (m.rigging) {
            case(Manipulator::Rigging::X_DIRECTION): {
                m.vector = glm::vec3(1.0f, 0.0f, 0.0f);
            } break;
            case(Manipulator::Rigging::Y_DIRECTION): {
                m.vector = glm::vec3(0.0f, 1.0f, 0.0f);
            } break;
            case(Manipulator::Rigging::Z_DIRECTION): {
                m.vector = glm::vec3(0.0f, 0.0f, 1.0f);
            } break;
            default: break;
        }
        if ((m.variety == Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION_USING_LOOKAT) &&
            (Manipulator::Rigging::VECTOR_DIRECTION)) {
            glm::vec4 view = snapshot.view_vector;
            m.vector = glm::vec3(view.x, view.y, view.z) * (-1.0f);
        }
        if ((m.variety == Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_UP_VECTOR) &&
             (Manipulator::Rigging::ROTATION)){
            glm::vec4 up = snapshot.up_vector;
            m.vector = glm::vec3(up.x, up.y, up.z);
        }
        m.vector = glm::normalize(m.vector);
    }

    return true;
}


bool KeyframeManipulators::PushRendering(CinematicUtils &utils) {

    glm::vec4 manipulator_color;
    auto pos = this->state.cam_min_snapshot.position;
    glm::vec3 camera_position = glm::vec3(pos[0], pos[1], pos[2]);
    glm::vec3 normal;
    glm::vec3 keyframe_position;
    glm::vec3 manipulator_position;
    pos = this->state.selected_keyframe.GetCameraState().position;
    glm::vec3 selected_position = glm::vec3(pos[0], pos[1], pos[2]);

    // Push keyframe position selectors
    auto selector_count = this->selectors.size();
    for (size_t i = 0; i < selector_count; ++i) {
        manipulator_color = utils.Color(CinematicUtils::Colors::KEYFRAME);
        if (i == this->state.selected_index) {
            manipulator_color = utils.Color(CinematicUtils::Colors::KEYFRAME_SELECTED);
        }
        utils.PushPointPrimitive(this->selectors[i].vector, (2.0f * this->point_radius), camera_position, manipulator_color);
        if (this->state.selected_index == i) {
            normal = glm::normalize(camera_position - (this->selectors[i].vector - selected_position) / 2.0f);
            utils.PushLinePrimitive(this->selectors[i].vector, selected_position, this->line_width, normal, utils.Color(CinematicUtils::Colors::KEYFRAME_DRAGGED));
        }
    }

    if (this->state.selected_index >= 0) {
        // Push selected keyframe manipulators
        for (auto &m : this->manipulators) {
            if (m.show) {
                manipulator_color = this->getManipulatorColor(m, utils);
                keyframe_position = this->getManipulatorPosition(m);
                if (this->toggleOusideBbox) {
                    /// TODO manipulator_position = keyframe_position + m.vector * this->line_length;
                }
                else {
                    manipulator_position = keyframe_position + m.vector * this->line_length;
                }
                normal = glm::normalize(camera_position - (keyframe_position - manipulator_position)/2.0f);
                switch (m.variety) {
                    case(Manipulator::Variety::MANIPULATOR_FIRST_CTRLPOINT_POSITION):
                    case(Manipulator::Variety::MANIPULATOR_LAST_CTRLPOINT_POSITION): {
                        if (m.rigging == Manipulator::Rigging::X_DIRECTION) { // Draw only once
                            utils.PushLinePrimitive(selected_position, keyframe_position, this->line_width, normal, utils.Color(CinematicUtils::Colors::MANIPULATOR_CTRLPOINT));
                        }
                    } break;
                    case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_LOOKAT_VECTOR): {
                        if (m.rigging == Manipulator::Rigging::X_DIRECTION) { // Draw only once
                            utils.PushLinePrimitive(selected_position, keyframe_position, this->line_width, normal, utils.Color(CinematicUtils::Colors::MANIPULATOR_VECTOR));
                        }
                    } break;
                    default: break;
                }
                utils.PushPointPrimitive(manipulator_position, (2.0f * this->point_radius), camera_position, manipulator_color);
                utils.PushLinePrimitive(keyframe_position, manipulator_position, this->line_width, normal, manipulator_color);
            }
        }
    } else {
        // Push intermediate keyframe marker
        view::Camera_2 cam(this->state.selected_keyframe.GetCameraState());
        cam_type::snapshot_type snapshot;
        cam.take_snapshot(snapshot, megamol::core::thecam::snapshot_content::all);
        glm::vec4 snap_position = snapshot.position;
        glm::vec4 snap_view = snapshot.view_vector;
        glm::vec4 snap_up = snapshot.up_vector;
        selected_position = glm::vec3(snap_position.x, snap_position.y, snap_position.z);
        glm::vec3 selected_view = glm::vec3(snap_view.x, snap_view.y, snap_view.z);
        selected_view = glm::normalize(selected_view) * this->line_length;
        glm::vec3 selected_up = glm::vec3(snap_up.x, snap_up.y, snap_up.z);
        selected_up = glm::normalize(selected_up) * this->line_length;
        normal = glm::normalize(camera_position - selected_position);
        manipulator_color = utils.Color(CinematicUtils::Colors::KEYFRAME);
        utils.PushPointPrimitive(selected_position, (2.0f * this->point_radius) * (3.0f/4.0f), camera_position, manipulator_color);
        utils.PushLinePrimitive(selected_position, selected_position + selected_view, this->line_width, normal, utils.Color(CinematicUtils::Colors::MANIPULATOR_VECTOR));
        utils.PushLinePrimitive(selected_position, selected_position + selected_up , this->line_width, normal, utils.Color(CinematicUtils::Colors::MANIPULATOR_ROTATION));
    }

    return true;
}


int KeyframeManipulators::GetSelectedKeyframePositionIndex(float mouse_x, float mouse_y) {

    int index = -1;
    auto count = this->selectors.size();
    glm::vec2 mouse = glm::vec2(mouse_x, mouse_y);

    view::Camera_2 cam(this->state.cam_min_snapshot);
    cam_type::snapshot_type snapshot;
    cam.take_snapshot(snapshot, megamol::core::thecam::snapshot_content::up_vector);
    glm::vec4 snapshot_up = snapshot.up_vector;
    glm::vec3 cam_up = glm::vec3(snapshot_up.x, snapshot_up.y, snapshot_up.z);

    for (size_t i = 0; i < count; ++i) {
        if (this->selectors[i].show) {
            if (this->checkMousePointIntersection(this->selectors[i], mouse, cam_up)) {
                return i;
            }
        }
    }
    return index;
}


bool KeyframeManipulators::CheckForHitManipulator(float mouse_x, float mouse_y) {

    this->state.hit = nullptr;
    glm::vec2 mouse = glm::vec2(mouse_x, mouse_y);

    view::Camera_2 cam(this->state.cam_min_snapshot);
    cam_type::snapshot_type snapshot;
    cam.take_snapshot(snapshot, megamol::core::thecam::snapshot_content::up_vector);
    glm::vec4 snapshot_up = snapshot.up_vector;
    glm::vec3 cam_up = glm::vec3(snapshot_up.x, snapshot_up.y, snapshot_up.z);

    for (auto &m : this->manipulators) {
        if (m.show) {
            if (this->checkMousePointIntersection(m, mouse, cam_up)) {
                this->state.mouse = mouse;
                this->state.hit = std::make_shared<Manipulator>(m);
                return true;
            }
        }
    }
    return false;
}


bool KeyframeManipulators::ProcessHitManipulator(float mouse_x, float mouse_y) {

    if (this->state.hit == nullptr) return false;

    const float sensitivity = 0.001f;

    glm::vec3 keyframe_position = this->getManipulatorPosition((*this->state.hit));
    glm::vec3 manipualtor_position = keyframe_position + this->state.hit->vector * this->line_length;
    float world_length = this->line_length;

    glm::vec2 screenspace_keyframe_position = this->world2ScreenSpace(keyframe_position);
    glm::vec2 screenspace_manipulator_position = this->world2ScreenSpace(manipualtor_position);
    glm::vec2 screenspace_vector = screenspace_manipulator_position - screenspace_keyframe_position;
    float screenspace_length = glm::length(screenspace_vector);

    glm::vec2 mouse = glm::vec2(mouse_x, mouse_y);
    glm::vec2 mouse_vector = mouse - this->state.mouse;
    screenspace_vector = glm::normalize(screenspace_vector);
    float diff_screenspace_length = glm::dot(screenspace_vector, mouse_vector);

    float diff_world_length = world_length / screenspace_length  * diff_screenspace_length;
    glm::vec3 diff_world_vector = this->state.hit->vector * diff_world_length;

    auto camera_state = this->state.selected_keyframe.GetCameraState();
    megamol::core::view::Camera_2 camera(camera_state);
    glm::vec4 camera_position = camera.position();
    glm::vec4 new_camera_position = camera_position + glm::vec4(diff_world_vector.x, diff_world_vector.y, diff_world_vector.z, 1.0f);

    /// TODO positio nof LookAt

    switch (this->state.hit->rigging) {
    case(Manipulator::Rigging::X_DIRECTION): {
        camera.position(new_camera_position);
    } break;
    case(Manipulator::Rigging::Y_DIRECTION): {
        camera.position(new_camera_position);
    } break;
    case(Manipulator::Rigging::Z_DIRECTION): {
        camera.position(new_camera_position);
    } break;
    case(Manipulator::Rigging::VECTOR_DIRECTION): {
        camera.position(new_camera_position);
    } break;
    case(Manipulator::Rigging::ROTATION): {
        /// TODO 
    } break;
    default: break;
    }

    cam_type::minimal_state_type new_camera_state;
    camera.get_minimal_state(new_camera_state);
    this->state.selected_keyframe.SetCameraState(new_camera_state);

    this->state.mouse = mouse;

    return true;
}


glm::vec3 KeyframeManipulators::getManipulatorPosition(Manipulator &manipulator) {

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    switch (manipulator.variety) {
        case(Manipulator::Variety::SELECTOR_KEYFRAME_POSITION): {
            position = manipulator.vector;
        } break;
        case(Manipulator::Variety::MANIPULATOR_FIRST_CTRLPOINT_POSITION): {
            position = this->state.first_ctrl_point;
        } break;
        case(Manipulator::Variety::MANIPULATOR_LAST_CTRLPOINT_POSITION): {
            position = this->state.last_ctrl_point;
        } break;
        case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION): {
            auto pos = this->state.selected_keyframe.GetCameraState().position;
            position = glm::vec3(pos[0], pos[1], pos[2]);
        } break;
        case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_POSITION_USING_LOOKAT): {
            auto pos = this->state.selected_keyframe.GetCameraState().position;
            position = glm::vec3(pos[0], pos[1], pos[2]);
        } break;
        case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_LOOKAT_VECTOR): {
            auto pos = this->state.selected_keyframe.GetCameraState().position;
            position = glm::vec3(pos[0], pos[1], pos[2]);
            view::Camera_2 cam(this->state.selected_keyframe.GetCameraState());
            cam_type::snapshot_type snapshot;
            cam.take_snapshot(snapshot, megamol::core::thecam::snapshot_content::view_vector);
            glm::vec4 snap_view = snapshot.view_vector;
            snap_view = glm::normalize(snap_view);
            position = position + glm::vec3(snap_view.x, snap_view.y, snap_view.z) * (2.0f * this->line_length);
        } break;
        case(Manipulator::Variety::MANIPULATOR_SELECTED_KEYFRAME_UP_VECTOR): {
            auto pos = this->state.selected_keyframe.GetCameraState().position;
            position = glm::vec3(pos[0], pos[1], pos[2]);
        } break;
        default: break;
    }
    return position;
}


glm::vec4 KeyframeManipulators::getManipulatorColor(Manipulator &manipulator, CinematicUtils &utils) {

    glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    switch (manipulator.rigging) {
        case(Manipulator::Rigging::X_DIRECTION): {
            color = utils.Color(CinematicUtils::Colors::MANIPULATOR_X);
        } break;
        case(Manipulator::Rigging::Y_DIRECTION): {
            color = utils.Color(CinematicUtils::Colors::MANIPULATOR_Y);
        } break;
        case(Manipulator::Rigging::Z_DIRECTION): {
            color = utils.Color(CinematicUtils::Colors::MANIPULATOR_Z);
        } break;
        case(Manipulator::Rigging::VECTOR_DIRECTION): {
            color = utils.Color(CinematicUtils::Colors::MANIPULATOR_VECTOR);
        } break;
        case(Manipulator::Rigging::ROTATION): {
            color = utils.Color(CinematicUtils::Colors::MANIPULATOR_ROTATION);
        } break;
        default: break;
    }
    return color;
}


bool KeyframeManipulators::checkMousePointIntersection(Manipulator &manipulator, glm::vec2 mouse, glm::vec3 cam_up) {

    glm::vec3 manipulator_position;
    if (manipulator.variety == Manipulator::Variety::SELECTOR_KEYFRAME_POSITION) {
        manipulator_position = this->getManipulatorPosition(manipulator);
    }
    else {
        manipulator_position = this->getManipulatorPosition(manipulator) + manipulator.vector * this->line_length;
    }
    glm::vec3 manipulator_extent = manipulator_position + glm::normalize(cam_up)  * this->point_radius;

    glm::vec2 position_screenspace = this->world2ScreenSpace(manipulator_position);
    glm::vec2 extent_screenspace = this->world2ScreenSpace(manipulator_extent);
    float radius_screenspace = glm::length(position_screenspace - extent_screenspace);

    return (glm::length(position_screenspace - mouse) <= radius_screenspace);
}


glm::vec2 KeyframeManipulators::world2ScreenSpace(glm::vec3 vec) {

	glm::vec4 world = { vec.x, vec.y, vec.z, 1.0f };
    world = this->state.mvp * world;
    world = world / world.w;
    glm::vec2 screen;
    screen.x = (world.x + 1.0f) / 2.0f * this->state.viewport.x;
    screen.y = glm::abs(world.y - 1.0f) / 2.0f * this->state.viewport.y; // (flipped y-axis)

    return screen;
}
