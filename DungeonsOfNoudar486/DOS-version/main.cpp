#include <string>
#include <map>
#include <memory>
#include <math.h>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;

#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "IFileLoaderDelegate.h"
#include "CPlainFileLoader.h"

#include "NativeBitmap.h"
#include "Common.h"

#include "Vec2i.h"
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"
#include "WindowOperations.h"
#include "DOSHacks.h"

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags,
                     const char* file, int line) {
  return malloc( size );
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName,
                     int flags, unsigned debugFlags, const char* file, int line) {
  return malloc( size );
}

int main(int argc, char *argv[]) {
  auto fileLoader =  std::make_shared<Knights::CPlainFileLoader>("res\\");

  fileLoader->setFilenameTransformation( kDosLongFileNameTransformer );
  
  readMap( fileLoader );

  initWindow();
  setMainLoop();
  destroyWindow();
  return 0;
}
