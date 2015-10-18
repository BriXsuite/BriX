INCLUDE_DIRECTORIES(${CYCLUS_CORE_INCLUDE_DIRS} ${CYCLUS_CORE_TEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

# add lib
ADD_LIBRARY(brix structures.cc librarytools.cc generaltools.cc)
INSTALL(TARGETS brix
    LIBRARY DESTINATION lib/cyclus
    COMPONENT brix
    )
use_cyclus("BriX" "ReactorLite")
use_cyclus("BriX" "ReactorX")
install_cyclus_module("BriX" "")
TARGET_LINK_LIBRARIES(BriX brix)
TARGET_LINK_LIBRARIES(BriX_unit_tests brix)
#SET_TARGET_PROPERTIES(BriX brix PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

#INSTALL_CYCLUS_STANDALONE("ReactorLite" "ReactorLite" "BriX")