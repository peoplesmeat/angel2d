//
//  HudTextActor.h
//  ClientGame
//
//  Created by Bill Davis on 8/13/14.
//
//

#ifndef ClientGame_HudTextActor_h
#define ClientGame_HudTextActor_h

#include <iostream>


class HudTextActor : public Actor {
public:
    HudTextActor(String text, String nickname, int x, int y);
    void Render();
private:
    String _string;
    String _fontNickname;
};


#endif
