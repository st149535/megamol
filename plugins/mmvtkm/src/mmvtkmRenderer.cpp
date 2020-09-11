/*
 * mmvtkmDataRenderer.cpp
 *
 * Copyright (C) 2020 by VISUS (Universitaet Stuttgart)
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "mmvtkm/mmvtkmRenderer.h"
#include "mmvtkm/mmvtkmDataCall.h"
#include "vtkm/io/reader/VTKDataSetReader.h"

#include "mmcore/param/EnumParam.h"

//#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//#include "vtkm/cont/DeviceAdapter.h"


using namespace megamol;
using namespace megamol::mmvtkm;


/**
 * mmvtkmDataRenderer::mmvtmDataRenderer
 */
mmvtkmDataRenderer::mmvtkmDataRenderer(void)
    : Renderer3DModule_2()
    , vtkmDataCallerSlot_("getData", "Connects the vtkm renderer with a vtkm data source")
    , psColorTables_("colorTable", "Colortables specify the color range of the data")
    , vtkmDataSet_()
    , bounds_()
    , fieldName_("pointvar")
    , scene_()
    , mapper_()
    , canvas_()
    , vtkmCamera_()
    , colorArray_(nullptr)
    , canvasWidth_(0.f)
    , canvasHeight_(0.f)
    , canvasDepth_(0.f)
	, localUpdate_(false) 
{
    this->vtkmDataCallerSlot_.SetCompatibleCall<megamol::mmvtkm::mmvtkmDataCallDescription>();
    this->MakeSlotAvailable(&this->vtkmDataCallerSlot_);

	this->psColorTables_.SetParameter(new core::param::EnumParam(3));
    this->psColorTables_.Param<core::param::EnumParam>()->SetTypePair(0, "normal");
    for (int i = 0; i < colorTables_.size(); ++i) {
        this->psColorTables_.Param<core::param::EnumParam>()->SetTypePair(i, colorTables_[i]);
	}
    this->psColorTables_.SetUpdateCallback(&mmvtkmDataRenderer::setLocalUpdate);
    this->MakeSlotAvailable(&this->psColorTables_);
}


/**
* mmvtkmDataRenderer::setLocalUpdate
*/
bool mmvtkmDataRenderer::setLocalUpdate(core::param::ParamSlot& slot) {
    localUpdate_ = true;
	return true; 
}


/**
 * mmvtkmDataRenderer::~mmvtkmDataRenderer
 */
mmvtkmDataRenderer::~mmvtkmDataRenderer(void) { this->Release(); }


/**
 * mmvtkmDataRenderer::create
 */
bool mmvtkmDataRenderer::create() { return true; }


/**
 * mmvtkmDataRenderer::release
 */
void mmvtkmDataRenderer::release() {
    // Renderer3DModule::release();
}


/**
 * mmvtkmDataRenderer::Render
 */
bool mmvtkmDataRenderer::Render(core::view::CallRender3D_2& call) {
    mmvtkmDataCall* lhsVtkmDc = this->vtkmDataCallerSlot_.CallAs<mmvtkmDataCall>();

    if (lhsVtkmDc == nullptr) {
        core::utility::log::Log::DefaultLog.WriteError("In %s at line %d. lhsVtkmDc is nullptr.", __FILE__, __LINE__);
        return false;
    }


    if (!(*lhsVtkmDc)(0)) { // getData
        return false;
    }


    if (lhsVtkmDc->HasUpdate() || localUpdate_) {
        vtkmDataSet_ = lhsVtkmDc->GetDataSet();
        
        vtkm::cont::ColorTable colorTable(psColorTables_.Param<core::param::EnumParam>()->ValueString().PeekBuffer());
        // default cellname = "cells"
        // default coordinatesystem name = "coordinates"
        // default fieldname = "pointvar"
        vtkm::rendering::Actor actor(vtkmDataSet_->GetCellSet(0), vtkmDataSet_->GetCoordinateSystem(0),
            vtkmDataSet_->GetPointField("pointvar"),
            colorTable); // depending on dataset change to getCellField with according FrameID
        scene_ = vtkm::rendering::Scene();
        scene_.AddActor(actor); // makescene(...)

        vtkmCamera_.ResetToBounds(bounds_);

        // TOOD set fieldnames of vtk file as dropdown as parameter
        localUpdate_ = false;
    }


    // Camera setup and complete snapshot generation
    core::view::Camera_2 cam;
    call.GetCamera(cam);
    cam_type::snapshot_type snapshot;
    cam_type::matrix_type viewTemp, projTemp;
    cam.calc_matrices(snapshot, viewTemp, projTemp, core::thecam::snapshot_content::all);
    glm::vec4 camView = snapshot.view_vector;
    glm::vec4 camUp = snapshot.up_vector;
    glm::vec4 camPos = snapshot.position;


    // camera settings for vtkm
    canvasWidth_ = cam.resolution_gate().width();
    canvasHeight_ = cam.resolution_gate().height();
    canvas_ = vtkm::rendering::CanvasRayTracer(canvasWidth_, canvasHeight_);
    vtkm::Vec<vtkm::Float32, 3> lookAt(camView.x, camView.y, camView.z);
    vtkm::Vec<vtkm::Float32, 3> up(camUp.x, camUp.y, camUp.z);
    vtkm::Vec<vtkm::Float32, 3> position(camPos.x, camPos.y, camPos.z);
    vtkm::Float32 nearPlane = snapshot.frustum_near;
    vtkm::Float32 farPlane = snapshot.frustum_far;
    vtkm::Float32 fovY = cam.aperture_angle();


    try {
        // setup a camera and point it to towards the center of the input data
        vtkmCamera_.SetLookAt(lookAt);
        vtkmCamera_.SetViewUp(up);
        vtkmCamera_.SetClippingRange(nearPlane, farPlane);
        vtkmCamera_.SetFieldOfView(fovY);
        vtkmCamera_.SetPosition(position);


        // update actor, acutally just the field, each frame
        // store dynamiccellset and coordinatesystem after reading them in GetExtents
        vtkm::rendering::View3D view(scene_, mapper_, canvas_, vtkmCamera_);
        // required
        view.Initialize();
        view.Paint();
        // the canvas holds the buffer of the offscreen rendered image
        vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32, 4>> colorBuffer = view.GetCanvas().GetColorBuffer();


        // pulling out the c array from the buffer
        // which can just be rendered
        colorArray_ = colorBuffer.GetStorage().GetBasePointer();

    } catch (const std::exception& e) {
        core::utility::log::Log::DefaultLog.WriteError("In % s at line %d. \n", __FILE__, __LINE__);
        core::utility::log::Log::DefaultLog.WriteError(e.what());
        return false;
    }


    // Write the C array to an OpenGL buffer
    glDrawPixels((GLint)canvasWidth_, (GLint)canvasHeight_, GL_RGBA, GL_FLOAT, colorArray_);

    // SwapBuffers();

    return true;
}


/**
 * mmvtkmDataRenderer::GetExtents
 */
bool mmvtkmDataRenderer::GetExtents(core::view::CallRender3D_2& call) {
    mmvtkmDataCall* lhsVtkmDc = this->vtkmDataCallerSlot_.CallAs<mmvtkmDataCall>();
    if (lhsVtkmDc == nullptr) {
        core::utility::log::Log::DefaultLog.WriteError("In %s at line %d. lhsVtkmDc is nullptr.", __FILE__, __LINE__);
        return false;
    }

    megamol::core::view::CallRender3D_2* cr = &call;
    if (cr == nullptr) {
        core::utility::log::Log::DefaultLog.WriteError(
            "In %s at line %d. CallRender3D_2 is nullptr.", __FILE__, __LINE__);
        return false;
    }

    if (!(*lhsVtkmDc)(1)) { // getMetaData
        return false;
    }

    if (lhsVtkmDc->HasUpdate()) {
        bounds_ = lhsVtkmDc->GetBounds();

        cr->AccessBoundingBoxes().SetBoundingBox(
            bounds_.X.Min, bounds_.Y.Min, bounds_.Z.Min, bounds_.X.Max, bounds_.Y.Max, bounds_.Z.Max);
        cr->AccessBoundingBoxes().SetClipBox(
            bounds_.X.Min, bounds_.Y.Min, bounds_.Z.Min, bounds_.X.Max, bounds_.Y.Max, bounds_.Z.Max);
    }

    return true;
}