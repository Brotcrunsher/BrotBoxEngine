add_subdirectory(Example2DPhysicsBallPit)
add_subdirectory(Example2DPhysicsChainReaction)
add_subdirectory(Example3D)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(Example3DLineDraw)
endif()
if(BBE_RENDER_MODE STREQUAL OpenGL OR BBE_RENDER_MODE STREQUAL Emscripten)
  if(BBE_ADD_EXPERIMENTAL)
    add_subdirectory(ExampleBackroomsGenerator)
  endif()
endif()
add_subdirectory(ExampleBalloonGame)
add_subdirectory(ExampleConvexHull)
add_subdirectory(ExampleCurves)
add_subdirectory(ExampleDivergence)
add_subdirectory(ExampleDoublePendulum)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleFarCube)
endif()
add_subdirectory(ExampleFlowField)
add_subdirectory(ExampleFluidSimulation)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleGrid)
endif()
add_subdirectory(ExampleMandelbrot)
if(NOT EMSCRIPTEN) # Requires "double" in shader code.
  add_subdirectory(ExampleMandelbrotShader)
endif()
if(WIN32)
  add_subdirectory(ExampleMandelbrotSIMD)
endif()
add_subdirectory(ExampleNeuralRocket)
add_subdirectory(ExampleParticleGravity)
add_subdirectory(ExampleParticleLife)
add_subdirectory(ExamplePendulumChaosShader)
add_subdirectory(ExamplePhasmophobiaCheatSheet)
if(BBE_ADD_AUDIO AND WIN32)
  add_subdirectory(ExamplePixelObserver)
endif()
add_subdirectory(ExampleRotatingCubeIntersections)
add_subdirectory(ExampleRotatingRectangleIntersections)
add_subdirectory(ExampleSandGame)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleSimplePresentation)
endif()
add_subdirectory(ExampleSignedDistanceField)
add_subdirectory(ExampleSnake)
add_subdirectory(ExampleSnake3D)
if(BBE_ADD_AUDIO)
  add_subdirectory(ExampleSound)
  add_subdirectory(ExampleSoundDataSource)
endif()
add_subdirectory(ExampleStreamCountdown)
add_subdirectory(ExampleTemplate)
add_subdirectory(ExampleTesseract)
add_subdirectory(ExampleTextRendering)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleVectorField)
endif()
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleWaveFunctionCollapse)
endif()
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleMother)
endif()
add_subdirectory(ExampleLSystem)
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExamplePaint)
endif()
if(BBE_ADD_EXPERIMENTAL)
  add_subdirectory(ExampleTowerDefense)
endif()
