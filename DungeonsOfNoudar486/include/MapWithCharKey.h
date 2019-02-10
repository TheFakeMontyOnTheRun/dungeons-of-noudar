//
// Created by monty on 07-03-2018.
//
#pragma once

// std
#include <stdint.h> // int types (uint8_t...)

namespace odb {

template<typename T>
class MapWithCharKey
{
    //------------------------------------------------------------------------//
    // iVars                                                                  //
    //------------------------------------------------------------------------//
private:
    T    mMap    [256]; // @todo(stdmatt): Remove magic values...
    bool mPresent[256]; // @todo(stdmatt): Remove magic values...

    //------------------------------------------------------------------------//
    // CTOR / DTOR                                                            //
    //------------------------------------------------------------------------//
public:
    MapWithCharKey() {
        // @improve(stdmatt): We can do a memset here!
        for (uint8_t c = 0; c < 255; ++c) {
            mPresent[c] = false;
        }
    }

    //------------------------------------------------------------------------//
    // Public Methods                                                         //
    //------------------------------------------------------------------------//
public:
    bool present(const uint8_t key) const {
        return mPresent[key];
    }

    const T& get(const uint8_t key) const {
        return mMap[key];
    }

    void set(const uint8_t key, const T &value) {
        mMap    [key] = value;
        mPresent[key] = true;
    }

    void remove(const uint8_t key) {
        mPresent[key] = false;
    }

    void clear() {
        // @improve(stdmatt): We can do a memset here!
        for (uint8_t c = 0; c < 255; ++c) {
            mPresent[c] = false;
        }
    }

}; // class MapWithCharKey
} // namespace odb
