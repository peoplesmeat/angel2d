//
//  HudTextActor.cpp
//  ClientGame
//
//  Created by Bill Davis on 8/13/14.
//
//

#include "HudTextActor.h"

HudTextActor::HudTextActor(String text, String nickname, int x, int y) {
    this->_string = text;
    this->_fontNickname = nickname;
    this->SetPosition(x, y);
}

void HudTextActor::Render()
{
	glColor4f(_color.R, _color.G, _color.B, _color.A);
    
    DrawGameText(_string, _fontNickname, (int)_position.X, (int)_position.Y, 0.0);
    
}