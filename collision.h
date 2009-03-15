#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <set>
#include <vector>
#include <utility>
#include <iostream>

struct Point {
    Point();
    Point(float *x,float *y);
    virtual ~Point() {}; //make it polymorphic
    float *x,*y;
};

struct Area : public Point {
    Area();
    Area(float *x,float *y);

    virtual float left() const=0;
    virtual float right() const=0;
    virtual float top() const=0;
    virtual float bottom() const=0;
    virtual bool collide_with(const Point *point) const=0;

    typedef std::set<Point*> Points;
    Points colliding_x,colliding_y,colliding;
};

struct Rectangle : public Area {
    Rectangle();
    Rectangle(float *x,float *y,const float *w,const float *h);

    virtual float left() const;
    virtual float right() const;
    virtual float top() const;
    virtual float bottom() const;
    virtual bool collide_with(const Point *point) const;

    const float *w,*h;
};

//***********************************************************
class CollisionManager {
public:
    static CollisionManager *get();
    static void free();
    static void init(size_t nspace=2);
    void dump(std::ostream &os=std::cout) const;

    void resolve_collision();
    typedef std::set<Point*> Points;
    typedef std::set<Area*> Areas;
    typedef std::pair<Points,Areas> Space;
    typedef std::vector<Space> Spaces;
    Spaces spaces;
protected:
    CollisionManager(size_t nspace);
    ~CollisionManager();

};
    

#endif
