#ifndef knights2_CFalconKnight_H
#define knights2_CFalconKnight_H
namespace Knights {
    class CFalconKnight : public CActor {
    public:
        CFalconKnight(int aId);
        void update(std::shared_ptr <CMap> map) override;
    };
}
#endif
