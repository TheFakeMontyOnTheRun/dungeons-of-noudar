#include <string>
#include <map>
#include <memory>
#include <vector>
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
#include <array>

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

int main(int argc, char *argv[]) {
  auto fileLoader =  std::make_shared<Knights::CPlainFileLoader>("res\\");

  fileLoader->setFilenameTransformation( kDosLongFileNameTransformer );
  
  readMap( fileLoader, "tiles.properties" );

  initWindow();
  setMainLoop();
  destroyWindow();
  return 0;
}
