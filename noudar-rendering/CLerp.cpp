#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "CLerp.h"

namespace odb {
	CLerp::CLerp( long initialValue, long finalValue, long duration ) :
			mDelta( std::abs(finalValue - initialValue) ), mDuration( duration ), mInitialValue( initialValue ),   mEllapsed( 0 ) {
	}
	
	float CLerp::getValue( long ellapsed ) {
		return mInitialValue + std::min( ( ( mDelta * ellapsed ) / mDuration ), mDelta ); 
	}

	float CLerp::getFinalValue() {
		return mInitialValue + mDelta;
	}

	long CLerp::getEllapsed() {
		return mEllapsed;
	}

	float CLerp::getCurrentValue() {
		return getValue( mEllapsed );
	}

	void CLerp::update( long ms ) {
		mEllapsed += ms;
	}

	long CLerp::getDuration() {
		return mDuration;
	}

	bool CLerp::isFinished() {
		return mEllapsed >= mDuration;
	}
}
