/*
* CinematicUtils.cpp
*
* Copyright (C) 2019 by VISUS (Universitaet Stuttgart).
* Alle Rechte vorbehalten.
*/

#include "stdafx.h"
#include "CinematicUtils.h"


using namespace megamol::cinematic;


CinematicUtils::CinematicUtils(void) : core::view::RenderUtils()
    , font(megamol::core::utility::SDFFont::PRESET_ROBOTO_SANS)
    , font_size(20.0f)
    , init_once(false)
    , background_color(0.0f, 0.0f, 0.0f, 0.0f)
    , hotkey_window_setup_once (true) {

}


CinematicUtils::~CinematicUtils(void) {

}


bool CinematicUtils::Initialise(megamol::core::CoreInstance* core_instance) {

    if (core_instance == nullptr) {
        megamol::core::utility::log::Log::DefaultLog.WriteError(
            "Pointer to core isntance is nullptr. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
    }

    if (this->init_once) {
        megamol::core::utility::log::Log::DefaultLog.WriteWarn("Primitive rendering has already been initialized. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
    }

    // Initialise font
    if (!this->font.Initialise(core_instance)) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("Couldn't initialize the font. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    this->font.SetBatchDrawMode(true);

    // Initialise rendering
    if (!this->InitPrimitiveRendering(core_instance->ShaderSourceFactory())) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("Couldn't initialize primitive rendering. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    auto err = glGetError();
    if (err != GL_NO_ERROR) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("OpenGL Error: %i [%s, %s, line %d]\n ", err, __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    this->init_once = true;

    return true;
}


const glm::vec4 CinematicUtils::Color(CinematicUtils::Colors c) const {

    glm::vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f };

    switch (c) {
    case (CinematicUtils::Colors::BACKGROUND):
        color = this->background_color;
        break;
    case (CinematicUtils::Colors::FOREGROUND): {
        glm::vec4 foreground = { 1.0f, 1.0f, 1.0f, 1.0f };
        color = this->background_color;
        for (unsigned int i = 0; i < 3; i++) {
            foreground[i] -= color[i];
        }
        color = foreground;
    } break;
    case (CinematicUtils::Colors::KEYFRAME):
        color = { 0.7f, 0.7f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::KEYFRAME_DRAGGED):
        color = { 0.5f, 0.5f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::KEYFRAME_SELECTED):
        color = { 0.2f, 0.2f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::KEYFRAME_SPLINE):
        color = { 0.4f, 0.4f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MENU):
        color = { 0.0f, 0.0f, 0.5f, 1.0f };
        break;
    case (CinematicUtils::Colors::FONT):
        color = { 1.0f, 1.0f, 1.0f, 1.0f };
        if (CinematicUtils::lightness(this->background_color) > 0.5f) {
            color = { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        break;
    case (CinematicUtils::Colors::FONT_HIGHLIGHT):
        color = { 0.75f, 0.75f, 0.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::LETTER_BOX):
        color = { 1.0f, 1.0f, 1.0f, 1.0f };
        if (CinematicUtils::lightness(this->background_color) > 0.5f) {
            color = { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        break;
    case (CinematicUtils::Colors::FRAME_MARKER):
        color = { 1.0f, 0.6f, 0.6f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_X):
        color = { 1.0f, 0.0f, 0.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_Y):
        color = { 0.0f, 1.0f, 0.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_Z):
        color = { 0.0f, 0.0f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_VECTOR):
        color = { 0.0f, 1.0f, 1.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_ROTATION):
        color = { 1.0f, 1.0f, 0.0f, 1.0f };
        break;
    case (CinematicUtils::Colors::MANIPULATOR_CTRLPOINT):
        color = { 1.0f, 0.0f, 1.0f, 1.0f };
        break;
    default: break;
    }

    return color;
}


void CinematicUtils::PushMenu(const glm::mat4& ortho, const std::string& left_label, const std::string& middle_label, const std::string& right_label, glm::vec2 dim_vp) {

    const float menu_height = this->font_size;

    // Push menu background quad
    this->PushQuadPrimitive(glm::vec3(0.0f, dim_vp.y, 0.0f), glm::vec3(0.0f, dim_vp.y - menu_height, 0.0f), 
        glm::vec3(dim_vp.x, dim_vp.y - menu_height, 0.0f), glm::vec3(dim_vp.x, dim_vp.y, 0.0f), this->Color(CinematicUtils::Colors::MENU));

    // Push menu labels
    float vpWhalf = dim_vp.x / 2.0f;
    float new_font_size = this->font_size;
    float leftLabelWidth = this->font.LineWidth(this->font_size, left_label.c_str());
    float midleftLabelWidth = this->font.LineWidth(this->font_size, middle_label.c_str());
    float rightLabelWidth = this->font.LineWidth(this->font_size, right_label.c_str());
    while (((leftLabelWidth + midleftLabelWidth / 2.0f) > vpWhalf) || ((rightLabelWidth + midleftLabelWidth / 2.0f) > vpWhalf)) {
        new_font_size -= 0.5f;
        leftLabelWidth = this->font.LineWidth(new_font_size, left_label.c_str());
        midleftLabelWidth = this->font.LineWidth(new_font_size, middle_label.c_str());
        rightLabelWidth = this->font.LineWidth(new_font_size, right_label.c_str());
    }
    float textPosY = dim_vp.y - (menu_height / 2.0f) + (new_font_size / 2.0f);
    auto current_back_color = this->Color(CinematicUtils::Colors::BACKGROUND);
    this->SetBackgroundColor(this->Color(CinematicUtils::Colors::MENU));
    auto color = this->Color(CinematicUtils::Colors::FONT);

    this->font.DrawString(ortho, glm::value_ptr(color), 0.0f, textPosY, new_font_size, false, left_label.c_str(), megamol::core::utility::SDFFont::ALIGN_LEFT_TOP);
    this->font.DrawString(ortho, glm::value_ptr(color), (dim_vp.x - midleftLabelWidth) / 2.0f, textPosY, new_font_size, false, middle_label.c_str(), megamol::core::utility::SDFFont::ALIGN_LEFT_TOP);
    this->font.DrawString(ortho, glm::value_ptr(color), (dim_vp.x - rightLabelWidth), textPosY, new_font_size, false, right_label.c_str(), megamol::core::utility::SDFFont::ALIGN_LEFT_TOP);
    this->SetBackgroundColor(current_back_color);
}


void CinematicUtils::HotkeyWindow(bool& inout_show, const glm::mat4& ortho, glm::vec2 dim_vp) {

    if (inout_show) {
        if (this->hotkey_window_setup_once) {
            ImGui::SetNextWindowPos(ImVec2(0.0f, this->font_size));
            this->hotkey_window_setup_once = false;
        }
 
        auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
        if (ImGui::Begin("[Cinematic] HOTKEYS", &inout_show, flags)) {
            auto header_flags = ImGuiTreeNodeFlags_DefaultOpen;

            if (ImGui::CollapsingHeader("  GLOBAL", header_flags)) {
                ImGui::TextUnformatted("[Shift+a] Apply current settings to selected/new keyframe.");
                ImGui::TextUnformatted("[Shift+d] Delete selected keyframe.");
                ImGui::TextUnformatted("[Shift+s] Save keyframes to file.");
                ImGui::TextUnformatted("[Shift+l] Load keyframes from file.");
                ImGui::TextUnformatted("[Shift+z] Undo keyframe changes.");
                ImGui::TextUnformatted("[Shift+y] Redo keyframe changes.");
            }

            if (ImGui::CollapsingHeader("  TRACKING SHOT", header_flags)) {
                ImGui::TextUnformatted("[Shift+q] Toggle different manipulators for the selected keyframe.");
                ImGui::TextUnformatted("[Shift+w] Show manipulators inside/outside of model bounding box.");
                ImGui::TextUnformatted("[Shift+u] Reset look-at vector of selected keyframe.");
            }

            if (ImGui::CollapsingHeader("  CINEMATIC", header_flags)) {
                ImGui::TextUnformatted("[Shift+r] Start/Stop rendering complete animation.");
                ImGui::TextUnformatted("[Shift+Space] Start/Stop animation preview.");
            }

            if (ImGui::CollapsingHeader("  TIMELINE", header_flags)) {
                ImGui::TextUnformatted("[Shift+Right/Left Arrow] Move selected keyframe on animation time axis.");
                ImGui::TextUnformatted("[Shift+f] Snap all keyframes to animation frames.");
                ImGui::TextUnformatted("[Shift+g] Snap all keyframes to simulation frames.");
                ImGui::TextUnformatted("[Shift+t] Linearize simulation time between two keyframes.");
                ImGui::TextUnformatted("[Shift+p] Reset shifted and scaled time axes.");
                ImGui::TextUnformatted("[Left Mouse Button] Select keyframe.");
                ImGui::TextUnformatted("[Middle Mouse Button] Axes scaling in mouse direction.");
                ImGui::TextUnformatted("[Right Mouse Button] Drag & drop keyframe / pan axes.");
                // ImGui::TextUnformatted("[Shift+v] Set same velocity between all keyframes (Experimental)."); ///XXX
                // Calcualation is not correct yet ...
            }
       
            ImGui::End();
        }
    }
}


void CinematicUtils::Push2DText(const glm::mat4& ortho, const std::string& text, float x, float y) {

    auto color = this->Color(CinematicUtils::Colors::FONT);
    this->font.DrawString(ortho, glm::value_ptr(color), x, y, this->font_size, false, text.c_str(), megamol::core::utility::SDFFont::ALIGN_LEFT_TOP);
}


void CinematicUtils::DrawAll(const glm::mat4&mvp, glm::vec2 dim_vp) {

    if (!this->init_once) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("Cinematic utilities must be initialized before drawing. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    this->DrawAllPrimitives(mvp, dim_vp);

    glDisable(GL_DEPTH_TEST);
    this->font.BatchDrawString(mvp);
    this->font.ClearBatchDrawCache();
    glEnable(GL_DEPTH_TEST);
}


float CinematicUtils::GetTextLineHeight(void) {

    return this->font.LineHeight(this->font_size);
}


float CinematicUtils::GetTextLineWidth(const std::string& text_line) {

    return this->font.LineWidth(this->font_size, text_line.c_str());
}


void CinematicUtils::SetTextRotation(float a, float x, float y, float z) {

    this->font.SetRotation(a, x, y, z);
}


const float CinematicUtils::lightness(glm::vec4 background) const {

    return ((glm::max(background[0], glm::max(background[1], background[2])) + glm::min(background[0], glm::min(background[1], background[2]))) / 2.0f);
}
