#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <set>
#include <vector>
#include <utility>
#include <iostream>

struct Point {
    virtual ~Point() {}; //make it polymorphic

    virtual float get_x() const=0;
    virtual float get_y() const=0;
};

struct Area : public Point {
    virtual float get_left() const=0;
    virtual float get_right() const=0;
    virtual float get_top() const=0;
    virtual float get_bottom() const=0;
    virtual bool collide_with(const Point *point) const=0;

    typedef std::set<Point*> Points;
    Points colliding_x,colliding_y,colliding;
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
