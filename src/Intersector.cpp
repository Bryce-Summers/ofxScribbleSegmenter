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
#include "Intersector.h"

namespace scrib {

    // Naive N^2 Intersection Algorithm.
    void Intersector::intersect_brute_force(std::vector<scrib::Line> * lines)
    {
        int numLines = lines->size();
        for (int a = 0; a < numLines; a++)
            for (int b = a + 1; b < numLines; b++)
            {
                lines->at(a).intersect(&(lines->at(b)));
            }
    }

    void Intersector::intersect(std::vector<scrib::Line> * lines)
    {
        EventPQ event_queue(lines);
        TupleBST tuple_bst;

        int len = event_queue.size();

        for (int i = 0; i < len; i++)
            //while(!event_queue.isEmpty())
        {
            Event event = event_queue.delMin();

            switch (event.type)
            {
            case Event::ENTER:

                tuple_bst.addTuple(event.tuple1);
                continue;

            case Event::EXIT:

                tuple_bst.removeTuple(event.tuple1);
                continue;
            }
        }

    }

    // ============================================================
    // Event Priority Queue Methods.
    // -----------------------------

    // -- Constructor.
    EventPQ::EventPQ(std::vector<scrib::Line> * lines)
    {
        int len = lines->size();

        for (int i = 0; i < len; i++)
        {
            scrib::Line * line = &(lines->at(i));

            Event enter;
            Event exit;

            ofPoint p1 = line->p1;
            ofPoint p2 = line->p2;

            // Enter at least x coordinate.
            // Exit at greatest x coordinate.
            // We are assuming that there are no vertical lines.
            if (p1.x < p2.x)
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
        exit.type = Event::EXIT;

        enter.x = p1.x;
        enter.y = p1.y;

        exit.x = p2.x;
        exit.y = p2.y;

        LineTuple * line_tuple = new LineTuple();
        line_tuple->x = p1.x;
        line_tuple->y = p1.y;
        line_tuple->line = line;
        line_tuple->slope = (p2.y - p1.y) / (p2.x - p1.x);

        enter.tuple1 = line_tuple;
        exit.tuple1 = line_tuple;
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

    void TupleBST::addTuple(LineTuple * line_tuple)
    {

        set<LineTuple *>::iterator iter = bst.begin();

        scrib::Line * line = line_tuple->line;

        while (iter != bst.end())
        {
            LineTuple * line_tuple = (*iter);
            scrib::Line * other_line = line_tuple->line;
            line->intersect(other_line);
            iter++;
        }

        bst.insert(line_tuple);
        return;

    }

    void TupleBST::removeTuple(LineTuple * line_tuple)
    {

        set<LineTuple *>::iterator iter = bst.find(line_tuple);

        // Not Found.
        if (iter == bst.end())
        {
            cout << "--Tuple Not Found!!! " << line_tuple->x << " " << line_tuple->y << " " << line_tuple->slope << endl;
            return;
        }
        else
        {
            //cout << "--Tuple Exited: " << line_tuple->x << " " << line_tuple->y << " " << line_tuple->slope << endl;
        }

        bst.erase(iter);

    }


}


