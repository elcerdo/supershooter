#include "collision.h"

#include "except.h"
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

Point::Point() {}
Point::Point(float *x,float *y) : x(x), y(y) {}
Area::Area() {}
Area::Area(float *x,float *y) : x(x), y(y), w(0), h(0) {}
float Area::left() const { return *x-w/2.; }
float Area::right() const { return *x+w/2.; }
float Area::top() const { return *y-h/2.; }
float Area::bottom() const {return *y+h/2.; }


//***********************************************************
struct OrderX { bool operator()(const Point *a,const Point *b) const { return *a->x < *b->x; } };
struct OrderY { bool operator()(const Point *a,const Point *b) const { return *a->y < *b->y; } };
struct OrderL { bool operator()(const Area *a,const Area *b) const { return a->left() < b->left(); } };
struct OrderB { bool operator()(const Area *a,const Area *b) const { return a->bottom() < b->bottom(); } };
struct OrderR { bool operator()(const Area *a,const Area *b) const { return a->right() < b->right(); } };
struct OrderU { bool operator()(const Area *a,const Area *b) const { return a->top() < b->top(); } };

typedef std::multiset<Point*,OrderX> OrderXPoints;
typedef std::multiset<Point*,OrderY> OrderYPoints;
typedef std::multiset<Area*,OrderL> OrderLAreas;
typedef std::multiset<Area*,OrderB> OrderBAreas;
typedef std::multiset<Area*,OrderR> OrderRAreas;
typedef std::multiset<Area*,OrderU> OrderUAreas;

//***********************************************************
static CollisionManager *mCollisionManager=NULL;

CollisionManager *CollisionManager::get() { return mCollisionManager; }
void CollisionManager::free() { if (mCollisionManager) { delete mCollisionManager; mCollisionManager=NULL; } }
void CollisionManager::init(size_t nspace) {
    if (mCollisionManager) throw Except(Except::SS_INIT_ERR);
    mCollisionManager=new CollisionManager(nspace);
}

CollisionManager::CollisionManager(size_t nspace) : spaces(nspace) {}
CollisionManager::~CollisionManager() {}

void CollisionManager::resolve_collision() {
    cout<<"******************************************"<<endl;
    for (Spaces::const_iterator ispace=spaces.begin(); ispace!=spaces.end(); ispace++) {
        const Points *points=&ispace->first;
        const Areas *areas=&ispace->second;

        cout<<points->size()<<" points "<<areas->size()<<" areas"<<endl;


        {
            for (Areas::const_iterator i=areas->begin(); i!=areas->end(); i++) (*i)->colliding_x.clear();

            OrderXPoints order_x_points;
            std::copy(points->begin(),points->end(),std::inserter(order_x_points,order_x_points.begin()));
            OrderLAreas order_l_areas;
            std::copy(areas->begin(),areas->end(),std::inserter(order_l_areas,order_l_areas.begin()));
            OrderRAreas order_r_areas;
            std::copy(areas->begin(),areas->end(),std::inserter(order_r_areas,order_r_areas.begin()));

            Areas inside;
            OrderLAreas::const_iterator il=order_l_areas.begin();
            OrderRAreas::const_iterator ir=order_r_areas.begin();
            for (OrderXPoints::const_iterator i=order_x_points.begin(); i!=order_x_points.end(); i++) {
                cout<<*(*i)->x<<": ";
                while (il!=order_l_areas.end() and (*il)->left()<*(*i)->x) {
                    inside.insert(*il);
                    cout<<(*il)->left()<<" ";
                    il++;
                }
                cout<<"| ";
                while (ir!=order_r_areas.end() and (*ir)->right()<*(*i)->x) {
                    inside.erase(*ir);
                    cout<<(*ir)->right()<<" ";
                    ir++;
                }
                cout<<endl;
                if (il==order_l_areas.end() and ir==order_r_areas.end()) break;

                for (Areas::const_iterator j=inside.begin(); j!=inside.end(); j++) (*j)->colliding_x.insert(*i);
            }

        }
        cout<<endl;
        {
            for (Areas::const_iterator i=areas->begin(); i!=areas->end(); i++) (*i)->colliding_y.clear();

            OrderYPoints order_y_points;
            std::copy(points->begin(),points->end(),std::inserter(order_y_points,order_y_points.begin()));
            OrderBAreas order_b_areas;
            std::copy(areas->begin(),areas->end(),std::inserter(order_b_areas,order_b_areas.begin()));
            OrderUAreas order_u_areas;
            std::copy(areas->begin(),areas->end(),std::inserter(order_u_areas,order_u_areas.begin()));

            Areas inside;
            OrderLAreas::const_iterator ib=order_b_areas.begin();
            OrderRAreas::const_iterator it=order_u_areas.begin();
            for (OrderXPoints::const_iterator i=order_y_points.begin(); i!=order_y_points.end(); i++) {
                cout<<*(*i)->x<<": ";
                while (it!=order_u_areas.end() and (*it)->top()<*(*i)->x) {
                    inside.insert(*it);
                    cout<<(*it)->top()<<" ";
                    it++;
                }
                cout<<"| ";
                while (ib!=order_b_areas.end() and (*ib)->bottom()<*(*i)->x) {
                    inside.erase(*ib);
                    cout<<(*ib)->bottom()<<" ";
                    ib++;
                }
                cout<<endl;
                if (ib==order_b_areas.end() and it==order_u_areas.end()) break;

                for (Areas::const_iterator j=inside.begin(); j!=inside.end(); j++) (*j)->colliding_y.insert(*i);
            }

        }
        cout<<endl;
        for (Areas::const_iterator i=areas->begin(); i!=areas->end(); i++) {
            (*i)->colliding.clear();
            std::set_intersection((*i)->colliding_x.begin(),(*i)->colliding_x.end(),(*i)->colliding_y.begin(),(*i)->colliding_y.end(),std::inserter((*i)->colliding,(*i)->colliding.begin()));
        }

        for (Areas::const_iterator i=areas->begin(); i!=areas->end(); i++) cout<<(*i)->colliding_x.size()<<" "<<(*i)->colliding_y.size()<<" "<<(*i)->colliding.size()<<endl;

    }

}

