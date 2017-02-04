//
// Created by monty on 22/10/16.
//
#include <string>
#include <memory>
#include <utility>
#include <iostream>

#include <utility>
#include "Vec2i.h"
#include "SplatAnimation.h"

void odb::SplatAnimation::update(long ms) {
	showSplatTime -= ms;
}

void odb::SplatAnimation::startSplatAnimation() {
	showSplatTime = TOTAL_ANIMATION_TIME;
}

int odb::SplatAnimation::getSplatFrame() {
	if (showSplatTime > 0) {
		return (int)((TOTAL_ANIMATION_TIME - showSplatTime) * NUMBER_OF_FRAMES) / TOTAL_ANIMATION_TIME;
	} else {
		return -1;
	}
}

bool odb::SplatAnimation::isFinished() {
	return showSplatTime <= 0;
}

odb::SplatAnimation::SplatAnimation(Knights::Vec2i aPosition ) : mPosition( aPosition ) {
	startSplatAnimation();
}

Knights::Vec2i odb::SplatAnimation::getPosition() {
	return mPosition;
}











