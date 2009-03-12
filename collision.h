#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <set>
#include <vector>
#include <utility>
#include <iostream>

struct Point {
    Point();
    Point(float *x,float *y);
    float *x,*y;

    virtual ~Point() {}; //make it polymorphic
};

struct Area {
    Area();
    Area(float *x,float *y);

    float left() const;
    float right() const;
    float top() const;
    float bottom() const;

    typedef std::set<Point*> Points;
    Points colliding_x,colliding_y,colliding;
    float *x,*y,w,h;

    virtual ~Area() {}
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
