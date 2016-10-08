#include <string>
#include <map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLES2/gl2.h>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <map>
#include <array>

#include "NativeBitmap.h"
#include "Common.h"
#include "GameNativeAPI.h"
#include "WindowOperations.h"

int main(int argc, char *argv[]) {
	initWindow();

	FILE *fd;

	fd = fopen("res/map_tiles0.txt", "r");
	readMap( readToString(fd) );
	fclose(fd);

    setMainLoop();
    //...
	destroyWindow();
	return 0;
}
