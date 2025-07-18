cmake_minimum_required(VERSION 3.12.0)
project(NCVC
    VERSION "4.15"
    DESCRIPTION "NC Viewer and Converter (Free CAM)"
    HOMEPAGE_URL "https://github.com/NCVC-CAM/NCVC"
    LANGUAGES CXX
)

set(CMAKE_MFC_FLAG 2)

set(NCVC_APP_SOURCES
    ${PROJECT_SOURCE_DIR}/NCVC/3dContourScanSetupDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/3dModelChild.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/3dModelDoc.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/3dModelView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/3dOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/3dRoughScanSetupDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/AboutDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/AddinDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/CADbindDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ChildBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/CustomControl.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/CustomMenu.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/CustomToolBar.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DocBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfAutoOutlineDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfAutoPocketDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFBlock.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFChild.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFdata.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFDoc.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfEditOrgDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFMakeClass.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFMakeOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfSetup3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DxfSetupReload.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFshape.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFShapeView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/DXFView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ExecOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ExecSetupDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ExtensionDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/FrameBuffer.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/GLSL.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/Layer.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/LayerDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup4.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MachineSetup5.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MainFrm.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MainStatusBar.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeBindOptDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeCustomCode.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeDXFDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeDXFDlg1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeDXFDlg2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup0.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup4.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeLatheSetup5.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlgEx.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlgEx11.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlgEx2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlgEx21.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCDlgEx3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup4.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup5.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup6.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNCSetup8.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeNurbsSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeWireSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeWireSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MakeWireSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/MyTemplate.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCChild.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCdata.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCdataGL.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCDoc.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCFindDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCInfoTab.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCInfoView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCJumpDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCListView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeLathe.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeLatheOpt.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeMill.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeMillOpt.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeWire.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCMakeWireOpt.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCVC.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCVCaddinDXF.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCVCaddinIF.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCVCaddinNC.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCView.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewGL.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewGL_Lathe.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewGL_Mill.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewGL_Wire.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewSplit.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewTab.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewXY.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewXZ.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCViewYZ.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCWorkDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCWorkDlg1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/NCWorkDlg2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/OBSdlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ReadDXF.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ShapePropDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/SplashWnd.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/stdafx.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TabViewBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ThreadDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ThumbnailDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_AutoWorkingSet.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_Correct.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_Cuttime.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_MakeLathe.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_MakeNCD.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_MakeNurbs.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_MakeWire.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_NCRead.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_ShapeSearch.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/TH_UVWire.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ToolBarSetupDlg.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewBase.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewBaseGL.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewOption.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup1.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup2.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup3.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup4.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup5.cpp
    ${PROJECT_SOURCE_DIR}/NCVC/ViewSetup6.cpp
)

add_executable(NCVC_APP WIN32 ${NCVC_APP_SOURCES})
target_compile_definitions(NCVC_APP PRIVATE _AFXDLL)

find_package(Boost 1.87.0 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(MFC REQUIRED)

set (libraries
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${MFC_LIBRARIES}
)
target_link_libraries(NCVC_APP ${libraries})
target_include_directories(NCVC_APP PRIVATE
    ${PROJECT_SOURCE_DIR}/NVCV
    ${PROJECT_SOURCE_DIR}/NVCV.SDK/include
    ${Boost_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${MFC_INCLUDE_DIRS}
)
