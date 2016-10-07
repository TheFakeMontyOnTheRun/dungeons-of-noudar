#ifndef knights2_CBullKnight_H
#define knights2_CBullKnight_H
namespace Knights {
    class CBullKnight : public CActor {
    public:
        CBullKnight(int aId);
        void update(std::shared_ptr <CMap> map) override;
    };
}
#endif
