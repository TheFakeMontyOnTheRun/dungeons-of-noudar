//
// Created by monty on 06/12/16.
//
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <map>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <array>
#include <stdio.h>
#include <cmath>

#include "NativeBitmap.h"
#include "Texture.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
#include "WavefrontOBJReader.h"
#include "Logger.h"

#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"


#include "VBORenderingJob.h"
#include "DungeonGLES2Renderer.h"
#include "LightningStrategy.h"
#include "IFileLoaderDelegate.h"
#include "CPlainFileLoader.h"
#include "GameNativeAPI.h"

#include "IRenderer.h"
#include "NativeBitmap.h"
#include "CKnight.h"


#include "CGame.h"
#include "Common.h"
#include "NoudarGLES2Bridge.h"
#include "SplatAnimation.h"

#include "NoudarDungeonSnapshot.h"