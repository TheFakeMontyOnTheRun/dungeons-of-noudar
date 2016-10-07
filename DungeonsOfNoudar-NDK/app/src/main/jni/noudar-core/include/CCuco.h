#ifndef knights2_CCuco_H
#define knights2_CCuco_H
namespace Knights {

    class CMap;

    class CCuco : public CActor, public std::enable_shared_from_this<CCuco>  {
    public:
        CCuco(int aId);

        void endOfTurn() override;

        void update(std::shared_ptr <CMap> map) override;

        bool actOn(int newX, int newY, std::shared_ptr <CMap> map);

        bool dealWith( std::shared_ptr<CMap> map, int x, int y );
    };
}
#endif
