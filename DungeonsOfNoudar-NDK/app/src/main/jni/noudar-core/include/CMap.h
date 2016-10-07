#ifndef knights2_CMap_H
#define knights2_CMap_H

namespace Knights {
    class CMap {
    private:
        std::shared_ptr<IMapElement> map[ 20 ][ 20 ];
        std::shared_ptr<CActor> mActors[ 20 ][ 20 ];
        bool block[ 20 ][ 20 ];
        char mElement[ 20 ][ 20 ];
        std::vector<std::shared_ptr<CActor>> actors;

        std::shared_ptr<CActor> mAvatar;
    public:
        bool isValid( int x, int y );
        bool isBlockAt( int x, int y );
        CMap( const std::string& data );
        void move( EDirection d, std::shared_ptr<CActor> a );
        bool attackIfNotFriendly( EDirection d, std::shared_ptr<CActor> a, bool mutual );
        void endOfTurn();
        std::shared_ptr<IMapElement> getMapElement( Vec2i position );
        char getElementAt( int x, int y );
        std::shared_ptr<CActor> getAvatar();
        std::vector<std::shared_ptr<CActor>> getActors();
        std::shared_ptr<CActor> getActorAt( Vec2i position );
        void setActorAt( Vec2i position, std::shared_ptr<CActor> actor );
        void moveActor( Vec2i from, Vec2i to, std::shared_ptr<CActor> actor );
        std::shared_ptr<CActor> attack( std::shared_ptr<CActor> a, Vec2i position , bool mutual );
    };
}
#endif
