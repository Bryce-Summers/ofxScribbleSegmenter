/*
 *  ofxSweepLine.h
 *
 *  Written by Bryce Summers on 7 - 23 - 2015.
 *
 *  Referenced: https://github.com/alexiswolfish/ofxSweepLine
 *
 *
 * Canonical scrib::Line objects are represented by LineTuple objects in a bst.
 * Events along the sweep line are managed by a priority queue.
 *
 * This implementation should properly handle lines originating from the same end points.
 * This implementation assumes that no lines have the same slope and no lines are vertical.
 *
 * Note : Sets regard types as being equal when their cooresponding ordering operator returns false reflexively.
 */

#pragma once

#include "ofMain.h"
#include <math.h>
#include <list>
#include <set>
#include "Line.h"

namespace scrib{

// Line Tuples are stored in a binary search tree to
// represent the lines currently crossing the sweep line.
class LineTuple
{

public:

    LineTuple()
    {
        line = NULL;
    }

    // Used to correctly order the y tuples by y coordinate.
    float x;
    float y;

    // The slope is mainly used to properly order lines originating from the same end point.
    float slope;

    // Every LineTuple is associated with one line.
    scrib::Line * line;
};

// Used to impose an ordering for the tuples in the bst.
struct LineTupleCompare
{
    // Returns true if e1 < e2.
    bool operator()(const LineTuple * e1, const LineTuple * e2) const
    {

        // Equal.
        if(e1 == e2)
        {
            return false;
        }

        if((e1 -> y) < (e2 -> y))
        {
            return true;
        }

        if((e2 -> y) < (e1 -> y))
        {
            return false;
        }

        // We want lines with greater slop to be higher in the bst.
        return (e1 -> slope) > (e2 -> slope);
    }
};


// The Binary Search Tree used to store the tuples.
class TupleBST
{

public:

    TupleBST(){};
    //virtual ~TupleBST(){};

    // IN the line_tuple.
    // OUT : The tuples that are above and below.
    void addTuple(LineTuple * line_tuple, LineTuple ** above, LineTuple ** below);
    void removeTuple(LineTuple * line_tuple, LineTuple ** above, LineTuple ** below);

private:
    set<LineTuple *, LineTupleCompare> bst;

};

// These objects represent events along the sweep line.
class Event
{
    public:

        Event()
        {
            tuple1 = NULL;
            tuple2 = NULL;
        }

        enum Type{ENTER, EXIT, INTERSECTION};

        Type type;

        float x;
        float y;

        LineTuple * tuple1;
        LineTuple * tuple2;

};

class EventPQ
{
    public:

        struct EventCompare
        {
            // Returns true if e1 < e2.
            bool operator()(const Event& e1, const Event& e2) const
            {
                // Equal.
                if(e1.tuple1 == e2.tuple1 &&
                   e1.tuple2 == e2.tuple2 &&
                   e1.type == e2.type)
                {
                    return false;
                }

                if(e1.x < e2.x){return true;}
                if(e2.x < e1.x){return false;}
                if(e1.y < e2.y){return true;}
                if(e2.y < e1.y){return false;}

                if((e1.type == Event::EXIT) && (e2.type == Event::ENTER))
                {
                    return true;
                }

                if((e1.tuple1->slope) > (e2.tuple1->slope))
                {
                    return true;
                }


                return false;
            }
        };

        // -- Constructor.
        // Takes a list of the initial lines and adds start and end events for each of them.
        EventPQ(std::vector<scrib::Line*> * lines);
        virtual ~EventPQ(){};

        std::set<Event, EventCompare> PQ;

        // Intersection Events are gurranteed to have higher segment as the first tuple before the intersection point.
        void addIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2);
        void removeIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2);

        Event delMin();
        bool isEmpty();

    private:

        void populateEvent(Event &enter, Event &exit, ofPoint &p1, ofPoint &p2, scrib::Line * line);
        void addOrRemoveIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2, bool should_remove);
};

class Intersector
{

public:
    Intersector(){};
    virtual ~Intersector(){};

    // Calls the Line::intersect method on all intersecting lines.
    // Does not treat lines that intersect at common points as intersecting.
    void intersect(std::vector<scrib::Line*> * lines);

private:
    // Adds neighbor events
    void neighbor_event(LineTuple * tuple1, LineTuple * tuple2,
                        EventPQ& event_queue);


    // Mutates the two tuples, such that their effective ordering in the binary tree will be reversed.
    void swap_tuples(LineTuple * tuple1, LineTuple * tuple2, float x, float y);

    // Returns true if an intersection event has occured before.
    // This is to prevent infinite loops.
    bool check_repeat_event(LineTuple * t1, LineTuple * t2, set<pair<LineTuple *, LineTuple *> > &event_set);
};

}
