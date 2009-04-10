
class StaticShip : public Ship {
public:
    StaticShip(Sprite *body,float ix,float iy) : Ship(body,100) {
        *x=ix;
        *y=iy;
    }
    bool move(float dt) { return true; }
};

class StaticArea : public Area {
public:
    StaticArea(Sprite *sprite) : Area(&sprite->x,&sprite->y), sprite(sprite) { w=sprite->w; h=sprite->h; }
    ~StaticArea() { delete sprite; }

    void draw() const { sprite->draw(); }
protected:
    Sprite *sprite;
};


