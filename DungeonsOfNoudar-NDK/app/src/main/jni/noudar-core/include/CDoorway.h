#ifndef knights2_CDoorway_H
#define knights2_CDoorway_H
namespace Knights {
    enum EDoorwayFunction {
        kEntry,
        kExit
    };

    class CDoorway : public IMapElement {
        const EDoorwayFunction doorFunction;
    public:
        CDoorway( EDoorwayFunction doorFunction );
        const EDoorwayFunction getDoorFunction();
    };
}
#endif
