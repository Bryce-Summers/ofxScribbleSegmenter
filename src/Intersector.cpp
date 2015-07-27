/*
 *  Intersector.cpp
 *
 *  Written by Bryce Summers on 7/23/2015.
 *
 *  Referenced: https://github.com/alexiswolfish/ofxSweepLine
 *
 */

#include <iostream>
#include <math.h>
#include "../include/Intersector.h"

namespace scrib{

void Intersector::intersect(std::vector<scrib::Line*> * lines)
{
    EventPQ event_queue(lines);
    TupleBST tuple_bst;

    set<pair<LineTuple *, LineTuple *> > intersection_events;

    while(!event_queue.isEmpty())
    {
        Event event = event_queue.delMin();

        LineTuple * above = NULL;
        LineTuple * below = NULL;

        // FIXME : Include the removal of intersection events.

        switch(event.type)
        {
            case Event::ENTER:

                std::cout << "ENTER Event" << endl;

                tuple_bst.addTuple(event.tuple1, &above, &below);

                if(above != NULL)
                {
                    neighbor_event(above, event.tuple1, event_queue);
                }

                if(below != NULL)
                {
                    neighbor_event(below, event.tuple1, event_queue);
                }


                continue;

            case Event::EXIT:

                std::cout << "EXIT Event" << endl;

                tuple_bst.removeTuple(event.tuple1, &above, &below);

                if(above != NULL && below != NULL)
                {
                    neighbor_event(above, below, event_queue);
                }

                // Conserve the Memory.
                //delete event.tuple1;

                continue;

            case Event::INTERSECTION:

                std::cout << "Intersection Event" << endl;
                cout << event.tuple1 << " " << event.tuple2 << endl;

                if(check_repeat_event(event.tuple1, event.tuple2, intersection_events))
                {
                    continue;
                }

                tuple_bst.removeTuple(event.tuple1, &above, &below);
                tuple_bst.removeTuple(event.tuple2, &above, &below);

                swap_tuples(event.tuple1, event.tuple2, event.x, event.y);

                // Note : The first line is the higher before and after the swap.

                // event.tuple1 is higher before the crossing.
                // event.tuple1 is lower after the crossing.
                // event.tuple2 is lower before the crossing.
                // event.tuple2 is higher after the crossing.

                if(above != NULL)
                {
                    event_queue.removeIntersectionEvent(above, event.tuple2);
                    neighbor_event(above, event.tuple1, event_queue);
                    //neighbor_event(above, event.tuple2, event_queue);
                }

                if(below != NULL)
                {
                    event_queue.removeIntersectionEvent(below, event.tuple1);
                    neighbor_event(below, event.tuple2, event_queue);
                    //neighbor_event(below, event.tuple1, event_queue);
                }

                tuple_bst.addTuple(event.tuple1, &above, &below);
                tuple_bst.addTuple(event.tuple2, &above, &below);

                continue;
        }
    }

    //cout << "End of Intersect" << endl;
}

bool Intersector::check_repeat_event(LineTuple * t1, LineTuple * t2, set<pair<LineTuple *, LineTuple *> > &intersection_events)
{
    pair<LineTuple *, LineTuple *> p1 = std::make_pair (t1, t2);
    pair<LineTuple *, LineTuple *> p2 = std::make_pair (t2, t1);

    set<pair<LineTuple *, LineTuple *> >::iterator iter;
    iter = intersection_events.find(p1);
    if (iter != intersection_events.end())
    {
        return true;
    }

    iter = intersection_events.find(p2);
    if (iter != intersection_events.end())
    {
        return true;
    }

    intersection_events.insert(p1);
    intersection_events.insert(p2);

    return false;
}

// This is where all of the intersections are processed.
void Intersector::neighbor_event(LineTuple * tuple1, LineTuple * tuple2,
                                 EventPQ& event_queue)
{

    scrib::Line * line1 = tuple1 -> line;
    scrib::Line * line2 = tuple2 -> line;

    // Generate an intersection event if necessary.
    if(line1 -> intersect(line2))
    {
        event_queue.addIntersectionEvent(tuple1, tuple2);
    }
}

// Swaps and mutates the tuples to start at the given intersection point.
void Intersector::swap_tuples(LineTuple * tuple1, LineTuple * tuple2, float x, float y)
{
    tuple1 -> x = x;
    tuple1 -> y = y;
    tuple2 -> x = x;
    tuple2 -> y = y;

    // We want these lineTuples to remain in the same part of memory,
    // so that they may be found in the Binary Search Tree.

    /*
    // Swap slopes.
    float temp_slope = tuple1 -> slope;
    tuple1 -> slope = tuple2  -> slope;
    tuple2 -> slope = temp_slope;

    // Swap line pointers.
    scrib::Line * temp_line = tuple1 -> line;
    tuple1 -> line = tuple2 -> line;
    tuple2 -> line = temp_line;
    */
}




// ============================================================
// Event Priority Queue Methods.
// -----------------------------

// -- Constructor.
EventPQ::EventPQ(std::vector<scrib::Line*> * lines)
{
    int len = lines -> size();

    for(int i = 0; i < len; i++)
    {
        scrib::Line * line = lines -> at(i);

        Event enter;
        Event exit;

        ofPoint p1 = line -> p1;
        ofPoint p2 = line -> p2;

        // Enter at least x coordinate.
        // Exit at greatest x coordinate.
        // We are assuming that there are no vertical lines.
        if(p1.x < p2.x)
        {
            populateEvent(enter, exit, p1, p2, line);
        }
        else
        {
            populateEvent(enter, exit, p2, p1, line);
        }

        PQ.insert(enter);
        PQ.insert(exit);

        //cout << "ENTER EVENT Generated : " << enter.x << ", " << enter.y << endl;
        //cout << "EXIT EVENT Generated : "  << exit.x  << ", " << exit.y  << endl;

    }

    //cout << endl;
}

void EventPQ::populateEvent(Event &enter, Event &exit, ofPoint &p1, ofPoint &p2, scrib::Line * line)
{
    enter.type = Event::ENTER;
    exit.type  = Event::EXIT;

    enter.x = p1.x;
    enter.y = p1.y;

    exit.x = p2.x;
    exit.y = p2.y;

    LineTuple * line_tuple = new LineTuple();
    line_tuple -> x = p1.x;
    line_tuple -> y = p1.y;
    line_tuple -> line = line;
    line_tuple -> slope = (p2.y - p1.y)/(p2.x - p1.x);

    enter.tuple1 = line_tuple;
    exit. tuple1 = line_tuple;
}

void EventPQ::addIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2)
{
    //cout << "Intersection Event Generated" << endl;
    addOrRemoveIntersectionEvent(tuple1, tuple2, false);
}

void EventPQ::removeIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2)
{
    addOrRemoveIntersectionEvent(tuple1, tuple2, true);
}

void EventPQ::addOrRemoveIntersectionEvent(LineTuple * tuple1, LineTuple * tuple2, bool should_remove)
{

    scrib::Line * line = tuple1 -> line;
    ofPoint intersection_point = line -> getLatestIntersectionPoint();

    Event event;

    event.type   = Event::INTERSECTION;
    event.x      = intersection_point.x;
    event.y      = intersection_point.y;

    // Impose an ordering on the tuples.
    if((tuple1->slope) < (tuple2->slope))
    {
        event.tuple1 = tuple1;
        event.tuple2 = tuple2;
    }
    else
    {
        event.tuple1 = tuple2;
        event.tuple2 = tuple1;
    }



    pair<set<Event>::iterator, bool> insert_result = PQ.insert(event);

    // Immediately remove the event (or the one that was already there).
    if(should_remove)
    {
        PQ.erase(insert_result.first);
    }
}

Event EventPQ::delMin()
{
    set<Event>::iterator iter = PQ.begin();
    Event output = *iter;
    PQ.erase(iter);
    return output;
}

bool EventPQ::isEmpty()
{
    return PQ.empty();
}


//=======================================================
// Binary Search Tree Representing the lines currently crossing the sweep line.
//--------------------------------

void TupleBST::addTuple(LineTuple * line_tuple, LineTuple ** above, LineTuple ** below)
{
    // Make sure not to give false positives.
    *above = NULL;
    *below = NULL;

    pair<set<LineTuple *>::iterator, bool> insert_result = bst.insert(line_tuple);

    //cout << "--Tuple Added: " << line_tuple->x << " " << line_tuple->y << " " << line_tuple->slope << endl;

    set<LineTuple *>::iterator iter = insert_result.first;

    if(iter != bst.begin())
    {
        iter--;// Move iterator backwards.
        *above = *iter;
        iter++;
    }

    iter++;
    if(iter != bst.end())
    {
        *below = *iter;
    }

}

void TupleBST::removeTuple(LineTuple * line_tuple, LineTuple ** above, LineTuple ** below)
{
    // Eliminate false positives.
    *above = NULL;
    *below = NULL;

    set<LineTuple *>::iterator iter = bst.find(line_tuple);

    // Not Found.
    if(iter == bst.end())
    {
        cout << "--Tuple Not Found!!! " << line_tuple->x << " " << line_tuple->y << " " << line_tuple->slope << endl;
        return;
    }
    else
    {
        //cout << "--Tuple Exited: " << line_tuple->x << " " << line_tuple->y << " " << line_tuple->slope << endl;
    }

    if(iter != bst.begin())
    {
        iter--;// Move iterator backwards.
        *above = *iter;
        iter++;
    }

    iter++;
    if(iter != bst.end())
    {
        *below = *iter;
    }

    // Finally go back and remove the desired line tuple from the bst.
    iter--;

    bst.erase(iter);

}


}


