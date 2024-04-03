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
    selectaccountdialog.h
    selectaccountdialog.cpp
    selectaccountdialog.ui
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
set(BudgetFace_SRCS
    ${backend_SRCS}
)
set(BudgetFaceUi_SRCS
    ${ui_SRCS}
    ${delegates_SRCS}
    ${dialogs_SRCS}
    ${widgets_SRCS}
)
if (FALSE)
    qt6_add_library(BudgetFaceLib STATIC ${BudgetFace_SRCS})
    add_library(BudgetFaceLib::BudgetFaceLib ALIAS BudgetFaceLib)
    target_compile_definitions(BudgetFaceLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
    target_include_directories(BudgetFaceLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    target_link_libraries(BudgetFaceLib PUBLIC
        QtModelUtilities::QtModelUtilities
        Qt::Core
        Qt::Sql
    )
    set_target_properties(BudgetFaceLib PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        VERSION ${VERSION_SHORT}
    )
    qt6_add_library(BudgetFaceUiLib STATIC ${BudgetFaceUi_SRCS})
    add_library(BudgetFaceUiLib::BudgetFaceUiLib ALIAS BudgetFaceUiLib)
    target_include_directories(BudgetFaceUiLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    target_link_libraries(BudgetFaceUiLib PUBLIC
        BudgetFaceLib::BudgetFaceLib
        Qt::Gui
        Qt::Widgets
    )
    set_target_properties(BudgetFaceUiLib PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        VERSION ${VERSION_SHORT}
    )
    qt6_add_executable(BudgetyMcBudgetface guimain.cpp)
    target_link_libraries(BudgetyMcBudgetface PUBLIC
        BudgetFaceUiLib::BudgetFaceUiLib
    )
else()
    qt6_add_executable(BudgetyMcBudgetface guimain.cpp ${BudgetFace_SRCS} ${BudgetFaceUi_SRCS})
    target_compile_definitions(BudgetyMcBudgetface PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
    target_include_directories(BudgetyMcBudgetface PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    target_link_libraries(BudgetyMcBudgetface PUBLIC
        QtModelUtilities::QtModelUtilities
        Qt::Core
        Qt::Sql
        Qt::Gui
        Qt::Widgets
    )
    set_target_properties(BudgetyMcBudgetface PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
endif()
set_target_properties(BudgetyMcBudgetface PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
install(TARGETS BudgetyMcBudgetface
    BUNDLE DESTINATION "BudgetyMcBudgetface"
    RUNTIME DESTINATION "BudgetyMcBudgetface"
)
qt_generate_deploy_app_script(
    TARGET BudgetyMcBudgetface
    OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
