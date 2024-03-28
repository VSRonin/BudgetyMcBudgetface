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
    accountstab.h
    accountstab.cpp
    accountstab.ui
    transactionstab.h
    transactionstab.cpp
    transactionstab.ui
    familytab.h
    familytab.cpp
    familytab.ui
)
set(dialogs_SRCS
    settingsdialog.h
    settingsdialog.cpp
    settingsdialog.ui
    addaccountdialog.h
    addaccountdialog.cpp
    addaccountdialog.ui
    addfamilymemberdialog.h
    addfamilymemberdialog.cpp
    addfamilymemberdialog.ui
)
set(widgets_SRCS
    multichoicecombo.h
    multichoicecombo.cpp
)
set(delegates_SRCS
    decimaldelegate.h
    decimaldelegate.cpp
    relationaldelegate.h
    relationaldelegate.cpp
    isodatedelegate.h
    isodatedelegate.cpp
    ownerdelegate.h
    ownerdelegate.cpp
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
    multiplefilterproxy.h
    multiplefilterproxy.cpp
    blankrowproxy.h
    blankrowproxy.cpp
)

source_group(UI FILES ${ui_SRCS})
source_group(Widgets FILES ${widgets_SRCS})
source_group(Delegates FILES ${delegates_SRCS})
source_group(Dialogs FILES ${dialogs_SRCS})
source_group(Backend FILES ${backend_SRCS})
set(TestBudget_SRCS
    ${backend_SRCS}
)
set(TestBudgetUi_SRCS
    ${ui_SRCS}
    ${delegates_SRCS}
    ${dialogs_SRCS}
    ${widgets_SRCS}
)
if (FALSE)
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
        TestBudgetUiLib::TestBudgetUiLib
    )
else()
    qt6_add_executable(TestBudget guimain.cpp ${TestBudget_SRCS} ${TestBudgetUi_SRCS})
    target_compile_definitions(TestBudget PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
    target_include_directories(TestBudget PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    target_link_libraries(TestBudget PUBLIC
        QtModelUtilities::QtModelUtilities
        Qt::Core
        Qt::Sql
        Qt::Gui
        Qt::Widgets
    )
    set_target_properties(TestBudget PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
endif()
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
