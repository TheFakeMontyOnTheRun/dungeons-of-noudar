//
// Created by monty on 14/01/17.
//

#ifndef DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
#define DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
namespace odb {

    class CTile3DProperties;

    using CTileId = uint8_t;

    using CTilePropertyMap = std::map< CTileId, CTile3DProperties >;

    using TextureName = std::string;

    enum GeometryType{ kNone, kCube, kLeftNearWall, kRightNearWall};

    class CTile3DProperties {
    public:
        bool mNeedsAlphaTest = true;
        TextureName mCeilingTexture;
        TextureName mFloorTexture;
        TextureName mMainWallTexture;
        GeometryType mGeometryType;
        TextureName mCeilingRepeatedWallTexture;
        TextureName mFloorRepeatedWallTexture;
        uint8_t mCeilingRepetitions;
        uint8_t mFloorRepetitions;
        FixP mCeilingHeight;
        FixP mFloorHeight;

        static CTilePropertyMap parsePropertyList( std::string propertyFile );
    };
}


#endif //DUNGEONSOFNOUDAR_NDK_TILEPROPERTIES_H
