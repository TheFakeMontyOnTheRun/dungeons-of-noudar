//
// Created by monty on 27/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H
#define DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H

bool setupGraphics(int w, int h, std::string vertexShader, std::string fragmentShader, std::vector<std::shared_ptr<odb::NativeBitmap>> textures );

void renderFrame(long delta);

void shutdown();

void updateAnimations( long delta );

void addCharacterMovement(int id, glm::vec2 previousPosition, glm::vec2 newPosition);

void updateCharacterMovements(const int *idsLocal);

void updateLevelSnapshot(const int *level, const int *actors, const int *splats);

void setCameraPosition( int x, int y );

void startFadingIn();

void startFadingOut();

bool isAnimating();

void rotateCameraLeft();

void rotateCameraRight();

void onReleasedLongPressingMove();

void onLongPressingMove();

void setEyeViewMatrix( float *eyeView );

void setPerspectiveMatrix( float *perspectiveMatrix );

void setAngleXZ( float XZAngle );

void setAngleYZ( float YZAngle );

void loadMapData( std::string geoFile, std::string petFile );

void readMap( std::string data );

void moveUp();

void moveDown();

void moveLeft();

void moveRight();

void gameLoopTick( long ms );

void setSoundEmitters( std::vector<std::shared_ptr<odb::SoundEmitter>> emitters, std::shared_ptr<odb::SoundListener> listener );

void forceDirection( int direction );

#endif //DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H
