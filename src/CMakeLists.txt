cmake_minimum_required(VERSION 3.14)
find_package(Qt6 6.3 COMPONENTS Widgets Gui Core Sql REQUIRED)
find_package(QtModelUtilities REQUIRED)
set(ui_SRCS 
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    centralwidget.cpp
    centralwidget.h
    centralwidget.ui
    decimaldelegate.h
    decimaldelegate.cpp
    settingsdialog.h
    settingsdialog.cpp
    settingsdialog.ui
    addaccountdialog.h
    addaccountdialog.cpp
    addaccountdialog.ui
)
set(backend_SRCS
    backendresources.qrc
    globals.h
    globals.cpp
    mainobject.h
    mainobject.cpp
    offlinesqltable.h
    offlinesqltable.cpp
    offlinesqlquerymodel.h
    offlinesqlquerymodel.cpp
)

source_group(UI FILES ${ui_SRCS})
source_group(Backend FILES ${backend_SRCS})
set(TestBudget_SRCS
    ${backend_SRCS}
)
set(TestBudgetUi_SRCS
    ${ui_SRCS}
)
qt6_add_library(TestBudgetLib STATIC ${TestBudget_SRCS})
add_library(TestBudgetLib::TestBudgetLib ALIAS TestBudgetLib)
target_compile_definitions(TestBudgetLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(TestBudgetLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(TestBudgetLib PUBLIC 
    QtModelUtilities::QtModelUtilities
    Qt::Core
    Qt::Sql
)
set_target_properties(TestBudgetLib PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
qt6_add_library(TestBudgetUiLib STATIC ${TestBudgetUi_SRCS})
add_library(TestBudgetUiLib::TestBudgetUiLib ALIAS TestBudgetUiLib)
target_include_directories(TestBudgetUiLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(TestBudgetUiLib PUBLIC
    TestBudgetLib::TestBudgetLib
    Qt::Gui
    Qt::Widgets
)
set_target_properties(TestBudgetUiLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
qt6_add_executable(TestBudget guimain.cpp)
target_link_libraries(TestBudget PUBLIC
    TestBudgetLib::TestBudgetLib
    TestBudgetUiLib::TestBudgetUiLib
)
set_target_properties(TestBudget PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
install(TARGETS TestBudget
    BUNDLE DESTINATION "TestBudget"
    RUNTIME DESTINATION "TestBudget"
)
qt_generate_deploy_app_script(
    TARGET TestBudget
    OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
