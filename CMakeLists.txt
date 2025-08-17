cmake_minimum_required(VERSION 3.16.0)
project(KRunnerWhatByte)

set(KF6_MIN_VERSION 6)
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# Find the required Libaries
find_package (Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Core Widgets)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS Runner)

set(krunner_whatbyte_SRCS
        srcs/sm.cpp
        srcs/krunnerwhatbyte.cpp
        srcs/helper.cpp
        )

kcoreaddons_add_plugin(krunner_whatbyte SOURCES ${krunner_whatbyte_SRCS} INSTALL_NAMESPACE "kf6/krunner")

target_link_libraries(krunner_whatbyte
                        KF6::Runner 
                        Qt6::Core Qt6::Widgets)

