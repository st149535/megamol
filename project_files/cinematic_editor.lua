﻿
fileToRender = "../project_files/testspheres.lua"
keyframeFile = "../project_files/cinematic_keyframes.kf"


function trafo(str)
  local newcontent = str:gsub("mmCreateView%(.-%)", "")
  newcontent = newcontent:gsub("mmCreateModule%(\"View.-%)", "")
  newcontent = newcontent:gsub("mmCreateCall%(\"CallRenderView.-%)", "")

  local viewname, viewclass, viewmoduleinst = str:match(
      '%(%s*[\"\']([^\"\']+)[\"\']%s*,%s*[\"\']([^\"\']+)[\"\']%s*,%s*[\"\']([^\"\']+)[\"\']%s*%)')

  print("viewname = " .. viewname)
  print("viewclass = " .. viewclass)
  print("viewmoduleinst = " .. viewmoduleinst)

  newcontent = newcontent:gsub('mmCreateCall%([\"\']CallRender3D[\'\"],%s*[\'\"]' 
      .. '.-' .. viewmoduleinst .. '::rendering[\'\"],([^,]+)%)', 'mmCreateCall("CallRender3D", "::ReplacementRenderer1::renderer",%1)'
      .. "\n" .. 'mmCreateCall("CallRender3D", "::ReplacementRenderer2::renderer",%1)')
  
  return newcontent
end

local content = mmReadTextFile(fileToRender, trafo)

print("read: " .. content)
code = load(content)
code()


mmCreateView("testeditor", "GUIView", "::GUIView1")
mmCreateModule("SplitView", "::SplitView1")
mmSetParamValue("::SplitView1::split.orientation", "1")
mmSetParamValue("::SplitView1::split.pos", "0.65")
mmSetParamValue("::SplitView1::split.colour", "gray")

mmCreateModule("SplitView", "::SplitView2")
mmSetParamValue("::SplitView2::split.pos", "0.55")
mmSetParamValue("::SplitView2::split.colour", "gray")

mmCreateModule("KeyframeKeeper", "::KeyframeKeeper1")
mmSetParamValue("::KeyframeKeeper1::storage::01_filename", keyframeFile)

mmCreateModule("View2D", "::View2D1")
mmSetParamValue("::View2D1::backCol", "black")
mmSetParamValue("::View2D1::resetViewOnBBoxChange", "True")

mmCreateModule("View3D", "::View3D1")
mmSetParamValue("::View3D1::backCol", "black")
mmSetParamValue("::View3D1::bboxCol", "gray")

mmCreateModule("TimeLineRenderer", "::TimeLineRenderer1")

mmCreateModule("CinematicRenderer", "::CinematicRenderer1")

mmCreateModule("CinematicView", "::CinematicView1")
mmSetParamValue("::CinematicView1::backCol", "grey")
mmSetParamValue("::CinematicView1::bboxCol", "white")
mmSetParamValue("::CinematicView1::07_fps", "24")
mmSetParamValue("::CinematicView1::stereo::projection", "2")

mmCreateModule("ReplacementRenderer", "::ReplacementRenderer1")
mmSetParamValue("::ReplacementRenderer1::03_replacmentKeyAssign", "6")
mmSetParamValue("::ReplacementRenderer1::01_replacementRendering", "on")

mmCreateModule("ReplacementRenderer", "::ReplacementRenderer2")
mmSetParamValue("::ReplacementRenderer2::03_replacmentKeyAssign", "5")
mmSetParamValue("::ReplacementRenderer2::01_replacementRendering", "off")


mmCreateCall("CallRenderView", "::GUIView1::renderview", "::SplitView1::render")
mmCreateCall("CallRenderView", "::SplitView1::render1", "::SplitView2::render")
mmCreateCall("CallRenderView", "::SplitView1::render2", "::View2D1::render")
mmCreateCall("CallRenderView", "::SplitView2::render1", "::View3D1::render")
mmCreateCall("CallCinematicCamera", "::TimeLineRenderer1::getkeyframes", "::KeyframeKeeper1::scene3D")
mmCreateCall("CallRender3D", "::View3D1::rendering", "::CinematicRenderer1::rendering")
mmCreateCall("CallCinematicCamera", "::CinematicRenderer1::keyframeKeeper", "::KeyframeKeeper1::scene3D")
mmCreateCall("CallRender3D", "::CinematicRenderer1::renderer", "::ReplacementRenderer1::rendering")
mmCreateCall("CallRender2D", "::View2D1::rendering", "::TimeLineRenderer1::rendering")
mmCreateCall("CallRenderView", "::SplitView2::render2", "::CinematicView1::render")
mmCreateCall("CallCinematicCamera", "::CinematicView1::keyframeKeeper", "::KeyframeKeeper1::scene3D")
mmCreateCall("CallRender3D", "::CinematicView1::rendering", "::ReplacementRenderer2::rendering")
