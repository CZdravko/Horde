INCLUDE_DIRECTORIES(
${BULLET_PHYSICS_SOURCE_DIR}/Extras/GIMPACT/include
${BULLET_PHYSICS_SOURCE_DIR}/src
${BULLET_PHYSICS_SOURCE_DIR}/Extras/GIMPACTUtils
${BULLET_PHYSICS_SOURCE_DIR}/Extras/ConvexDecomposition
)

ADD_LIBRARY(GIMPACTUtils
btGImpactConvexDecompositionShape.cpp	btGImpactConvexDecompositionShape.h
)

set_target_properties(GIMPACTUtils PROPERTIES COMPILE_FLAGS -fPIC)

IF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES(GIMPACTUtils ConvexDecomposition BulletCollision)
ENDIF (BUILD_SHARED_LIBS)
