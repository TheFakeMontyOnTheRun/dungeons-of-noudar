#ifndef knights2_CTurtleKnight_H
#define knights2_CTurtleKnight_H
namespace Knights {
    class CTurtleKnight : public CActor {
    public:
        CTurtleKnight(int aId);
        void update(std::shared_ptr <CMap> map) override;
    };
}
#endif
