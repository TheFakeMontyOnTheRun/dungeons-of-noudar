//
// Created by monty on 27/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H
#define DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H

bool setupGraphics(int w, int h, std::string vertexShader, std::string fragmentShader, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader );

void renderFrame(long delta);

void shutdown();

void updateAnimations( long delta );

void addCharacterMovement(int id, glm::vec2 previousPosition, glm::vec2 newPosition);

void setSnapshot(const odb::NoudarDungeonSnapshot& snapshot );

void startFadingIn();

void startFadingOut();

bool isAnimating();

void rotateCameraLeft();

void rotateCameraRight();

void setEyeViewMatrix( float *eyeView );

void setPerspectiveMatrix( float *perspectiveMatrix );

void setAngleXZ( float XZAngle );

void setAngleYZ( float YZAngle );

void loadMapData( std::string geoFile, std::string petFile );

void readMap( std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate, std::string tilePropertiesFile );

void moveUp();

void moveDown();

void moveLeft();

void moveRight();

void pickupItem();

void dropItem();

void cycleNextItem();

void cyclePrevItem();

void gameLoopTick( long ms );

void setSoundEmitters( std::vector<std::shared_ptr<odb::SoundEmitter>> emitters, std::shared_ptr<odb::SoundListener> listener );

void loadMeshList( std::vector< std::string> meshes, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate );

void forceDirection( int direction );

void setTileProperties( std::string tileProperties );

void interact();

bool isThereAnyObjectInFrontOfYou();
#endif //DUNGEONSOFNOUDAR_NDK_GAMENATIVEAPI_H
